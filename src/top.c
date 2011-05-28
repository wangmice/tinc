/*
    top.c -- Show real-time statistics from a running tincd
    Copyright (C) 2011 Guus Sliepen <guus@tinc-vpn.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "system.h"

#include <curses.h>

#include "control_common.h"
#include "list.h"
#include "tincctl.h"
#include "top.h"
#include "xalloc.h"

typedef struct nodestats_t {
	char *name;
	uint64_t in_packets;
	uint64_t in_bytes;
	uint64_t out_packets;
	uint64_t out_bytes;
	float in_packets_rate;
	float in_bytes_rate;
	float out_packets_rate;
	float out_bytes_rate;
	bool known;
} nodestats_t;

static const char *const sortname[] = {
	"name",
	"in pkts",
	"in bytes",
	"out pkts",
	"out bytes",
	"tot pkts",
	"tot bytes",
};

static int sortmode = 0;
static bool cumulative = false;

static list_t node_list;
static struct timeval now, prev, diff;
static int delay = 1000;
static bool running = true;

static void update(int fd) {
	sendline(fd, "%d %d", CONTROL, REQ_DUMP_TRAFFIC);
	gettimeofday(&now, NULL);

	timersub(&now, &prev, &diff);
	prev = now;
	float interval = diff.tv_sec + diff.tv_usec * 1e-6;

	char line[4096];
	char name[4096];
	int code;
	int req;
	uint64_t in_packets;
	uint64_t in_bytes;
	uint64_t out_packets;
	uint64_t out_bytes;

	for(list_node_t *i = node_list.head; i; i = i->next) {
		nodestats_t *node = i->data;
		node->known = false;
	}

	while(recvline(fd, line, sizeof line)) {
		int n = sscanf(line, "%d %d %s %"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64, &code, &req, name, &in_packets, &in_bytes, &out_packets, &out_bytes);

		if(n == 2)
			break;

		if(n != 7) {
			endwin();
			fprintf(stderr, "Error receiving traffic information\n");
			exit(1);
		}

		nodestats_t *found = NULL;

		for(list_node_t *i = node_list.head; i; i = i->next) {
			nodestats_t *node = i->data;
			int result = strcmp(name, node->name);
			if(result > 0) {
				continue;
			} if(result == 0) {
				found = node;
				break;
			} else {
				found = xmalloc_and_zero(sizeof *found);
				found->name = xstrdup(name);
				list_insert_after(&node_list, i, found);
				break;
			}
		}

		if(!found) {
			found = xmalloc_and_zero(sizeof *found);
			found->name = xstrdup(name);
			list_insert_tail(&node_list, found);
		}

		found->known = true;
		found->in_packets_rate = (in_packets - found->in_packets) / interval;
		found->in_bytes_rate = (in_bytes - found->in_bytes) / interval;
		found->out_packets_rate = (out_packets - found->out_packets) / interval;
		found->out_bytes_rate = (out_bytes - found->out_bytes) / interval;
		found->in_packets = in_packets;
		found->in_bytes = in_bytes;
		found->out_packets = out_packets;
		found->out_bytes = out_bytes;
	}
}

static void redraw(void) {
	erase();

	mvprintw(0, 0, "Tinc %-16s  Nodes: %4d  Sort: %-8s  %s", netname, node_list.count, sortname[sortmode], cumulative ? "Cumulative" : "Current");
	attrset(A_REVERSE);
	mvprintw(2, 0, "Node                IN pkts   IN bytes   OUT pkts  OUT bytes");
	chgat(-1, A_REVERSE, 0, NULL);

	nodestats_t *sorted[node_list.count];
	int n = 0;
	for(list_node_t *i = node_list.head; i; i = i->next)
		sorted[n++] = i->data;
	
	int cmpfloat(float a, float b) {
		if(a < b)
			return -1;
		else if(a > b)
			return 1;
		else
			return 0;
	}

	int cmpu64(uint64_t a, uint64_t b) {
		if(a < b)
			return -1;
		else if(a > b)
			return 1;
		else
			return 0;
	}

	int sortfunc(const void *a, const void *b) {
		const nodestats_t *na = *(const nodestats_t **)a;
		const nodestats_t *nb = *(const nodestats_t **)b;
		switch(sortmode) {
			case 1:
				if(cumulative)
					return -cmpu64(na->in_packets, nb->in_packets);
				else
					return -cmpfloat(na->in_packets_rate, nb->in_packets_rate);
			case 2:
				if(cumulative)
					return -cmpu64(na->in_bytes, nb->in_bytes);
				else
					return -cmpfloat(na->in_bytes_rate, nb->in_bytes_rate);
			case 3:
				if(cumulative)
					return -cmpu64(na->out_packets, nb->out_packets);
				else
					return -cmpfloat(na->out_packets_rate, nb->out_packets_rate);
			case 4:
				if(cumulative)
					return -cmpu64(na->out_bytes, nb->out_bytes);
				else
					return -cmpfloat(na->out_bytes_rate, nb->out_bytes_rate);
			case 5:
				if(cumulative)
					return -cmpu64(na->in_packets + na->out_packets, nb->in_packets + nb->out_packets);
				else
					return -cmpfloat(na->in_packets_rate + na->out_packets_rate, nb->in_packets_rate + nb->out_packets_rate);
			case 6:
				if(cumulative)
					return -cmpu64(na->in_bytes + na->out_bytes, nb->in_bytes + nb->out_bytes);
				else
					return -cmpfloat(na->in_bytes_rate + na->out_bytes_rate, nb->in_bytes_rate + nb->out_bytes_rate);
			default:
				return strcmp(na->name, nb->name);
		}
	}

	qsort(sorted, n, sizeof *sorted, sortfunc);

	int row = 3;
	for(int i = 0; i < n; i++, row++) {
		nodestats_t *node = sorted[i];
		if(node->known)
			if(node->in_packets_rate || node->out_packets_rate)
				attrset(A_BOLD);
			else
				attrset(A_NORMAL);
		else
			attrset(A_DIM);

		if(cumulative)
			mvprintw(row, 0, "%-16s %'10"PRIu64" %'10"PRIu64" %'10"PRIu64" %'10"PRIu64,
					node->name, node->in_packets, node->in_bytes, node->out_packets, node->out_bytes);
		else
			mvprintw(row, 0, "%-16s %'10.0f %'10.0f %'10.0f %'10.0f",
					node->name, node->in_packets_rate, node->in_bytes_rate, node->out_packets_rate, node->out_bytes_rate);
	}

	attrset(A_NORMAL);
	move(1, 0);

	refresh();
}

void top(int fd) {
	initscr();
	timeout(delay);

	while(running) {
		update(fd);
		redraw();

		switch(getch()) {
			case 's': {
				timeout(-1);
				float input = delay * 1e-3;
				printw("Change delay from %.1fs to: ", input);
				scanw("%f", &input);
				if(input < 0.1)
					input = 0.1;
				delay = input * 1e3;
				timeout(delay);
				break;
			}
			case 'c':
				  cumulative = !cumulative;
				  break;
			case 'n':
				  sortmode = 0;
				  break;
			case 'i':
				  sortmode = 2;
				  break;
			case 'I':
				  sortmode = 1;
				  break;
			case 'o':
				  sortmode = 4;
				  break;
			case 'O':
				  sortmode = 3;
				  break;
			case 't':
				  sortmode = 6;
				  break;
			case 'T':
				  sortmode = 5;
				  break;
			case 'q':
			case 27:
			case KEY_BREAK:
				running = false;
				break;
			default:
				break;
		}
	}

	endwin();
}
