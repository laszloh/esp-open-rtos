/* 
 *  Squeezelite - lightweight headless squeezebox emulator
 *
 *  (c) Adrian Smith 2012-2015, triode1@btinternet.com
 *      Ralph Irving 2015-2017, ralph_irving@hotmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "squeezelite.h"
#include "lwip/sockets.h"
#include "netdb.h"

// logging functions
const char *logtime(void) {
	static char buf[100];
	struct timeval tv;
	gettimeofday(&tv, NULL);
	strftime(buf, sizeof(buf), "[%T.", localtime(&tv.tv_sec));
	sprintf(buf+strlen(buf), "%06ld]", (long)tv.tv_usec);
	return buf;
}

void logprint(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	fflush(stderr);
}

// cmdline parsing
char *next_param(char *src, char c) {
	static char *str = NULL;
	char *ptr, *ret;
	if (src) str = src;
	if (str && (ptr = strchr(str, c))) {
		ret = str;
		*ptr = '\0';
		str = ptr + 1;
	} else {
		ret = str;
		str = NULL;
	}

	return ret && ret[0] ? ret : NULL;
}

// clock
u32_t gettime_ms(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


void get_mac(u8_t mac[]) {
	sdk_wifi_get_macaddr(STATION_IF, mac);
}

void set_nonblock(sockfd s) {
	int flags = fcntl(s, F_GETFL,0);
	fcntl(s, F_SETFL, flags | O_NONBLOCK);
}

// connect for socket already set to non blocking with timeout in seconds
int connect_timeout(sockfd sock, const struct sockaddr *addr, socklen_t addrlen, int timeout) {
	fd_set w, e;
	struct timeval tval;

	if (connect(sock, addr, addrlen) < 0) {
		if (last_error() != EINPROGRESS) {
			return -1;
		}
	}

	FD_ZERO(&w);
	FD_SET(sock, &w);
	e = w;
	tval.tv_sec = timeout;
	tval.tv_usec = 0;

	// only return 0 if w set and sock error is zero, otherwise return error code
	if (select(sock + 1, NULL, &w, &e, timeout ? &tval : NULL) == 1 && FD_ISSET(sock, &w)) {
		int	error = 0;
		socklen_t len = sizeof(error);
		getsockopt(sock, SOL_SOCKET, SO_ERROR, (void *)&error, &len);
		return error;
	}

	return -1;
}

void server_addr(char *server, in_addr_t *ip_ptr, unsigned *port_ptr) {
	struct addrinfo *res = NULL;
	struct addrinfo hints;
	const char *port = NULL;
	
	if (strtok(server, ":")) {
		port = strtok(NULL, ":");
		if (port) {
			*port_ptr = atoi(port);
		}
	}

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	
	getaddrinfo(server, NULL, &hints, &res);
	
	if (res && res->ai_addr) {
		*ip_ptr = ((struct sockaddr_in*)res->ai_addr)->sin_addr.s_addr;
	} 
	
	if (res) {
		freeaddrinfo(res);
	}
}

// pack/unpack to network byte order
void packN(u32_t *dest, u32_t val) {
	u8_t *ptr = (u8_t *)dest;
	*(ptr)   = (val >> 24) & 0xFF; *(ptr+1) = (val >> 16) & 0xFF; *(ptr+2) = (val >> 8) & 0xFF;	*(ptr+3) = val & 0xFF;
}

void packn(u16_t *dest, u16_t val) {
	u8_t *ptr = (u8_t *)dest;
	*(ptr) = (val >> 8) & 0xFF; *(ptr+1) = val & 0xFF;
}

u32_t unpackN(u32_t *src) {
	u8_t *ptr = (u8_t *)src;
	return *(ptr) << 24 | *(ptr+1) << 16 | *(ptr+2) << 8 | *(ptr+3);
} 

u16_t unpackn(u16_t *src) {
	u8_t *ptr = (u8_t *)src;
	return *(ptr) << 8 | *(ptr+1);
} 


// this only implements numfds == 1
int poll(struct pollfd *fds, unsigned long numfds, int timeout) {
	fd_set r, w;
	struct timeval tv;
	int ret;
	
	FD_ZERO(&r);
	FD_ZERO(&w);
	
	if (fds[0].events & POLLIN) FD_SET(fds[0].fd, &r);
	if (fds[0].events & POLLOUT) FD_SET(fds[0].fd, &w);
	
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = 1000 * (timeout % 1000);
	
	ret = select(fds[0].fd + 1, &r, &w, NULL, &tv);

	if (ret < 0) return ret;
	
	fds[0].revents = 0;
	if (FD_ISSET(fds[0].fd, &r)) fds[0].revents |= POLLIN;
	if (FD_ISSET(fds[0].fd, &w)) fds[0].revents |= POLLOUT;
	
	return ret;
}

