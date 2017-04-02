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
 * Additions (c) Paul Hermann, 2015-2017 under the same license terms
 *   -Control of Raspberry pi GPIO for amplifier power
 *   -Launch script on power status change from LMS
 */

// make may define: PORTAUDIO, SELFPIPE, RESAMPLE, RESAMPLE_MP, VISEXPORT, GPIO, IR, DSD, LINKALL to influence build

#define VERSION "v1.8.6-945"

#if !defined(MODEL_NAME)
#define MODEL_NAME SqueezeLite
#endif

#define QUOTE(name) #name
#define STR(macro)  QUOTE(macro)
#define MODEL_NAME_STRING STR(MODEL_NAME)

// The codecs and outputs
#if defined(RESAMPLE) || defined(RESAMPLE_MP)
#undef  RESAMPLE
#define RESAMPLE  1 // resampling
#define PROCESS   1 // any sample processing (only resampling at present)
#else
#define RESAMPLE  0
#define PROCESS   0
#endif
#if defined(RESAMPLE_MP)
#undef RESAMPLE_MP
#define RESAMPLE_MP 1
#else
#define RESAMPLE_MP 0
#endif

#if defined(IR)
#undef IR
#define IR 1
#else
#define IR 0
#endif

#if defined(SQUEEZE_GPIO)
#undef SQUEEZE_GPIO
#define SQUEEZE_GPIO 1
#else
#define SQUEEZE_GPIO 0
#endif

#if defined(FLAC)
#undef FLAC
#define FLAC 1
#else
#define FLAC 0
#endif

#if defined(MAD)
#undef MAD
#define MAD 1
#else
#define MAD 0
#endif

#if defined(SW_GAIN)
#undef SW_GAIN
#define SW_GAIN 1
#else
#define SW_GAIN 0
#endif

#define MAX_SILENCE_FRAMES 512

#define FIXED_ONE 0x10000

#define BYTES_PER_FRAME 8

#define min(a,b) (((a) < (b)) ? (a) : (b))

// logging
typedef enum { lERROR = 0, lWARN, lINFO, lDEBUG, lSDEBUG } log_level;

const char *logtime(void);
void logprint(const char *fmt, ...);

#define LOG_ERROR(fmt, ...) logprint("%s %s:%d " fmt "\n", logtime(), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  if (loglevel >= lWARN)  logprint("%s %s:%d " fmt "\n", logtime(), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  if (loglevel >= lINFO)  logprint("%s %s:%d " fmt "\n", logtime(), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) if (loglevel >= lDEBUG) logprint("%s %s:%d " fmt "\n", logtime(), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_SDEBUG(fmt, ...) if (loglevel >= lSDEBUG) logprint("%s %s:%d " fmt "\n", logtime(), __FUNCTION__, __LINE__, ##__VA_ARGS__)

// config options
#define STREAMBUF_SIZE (16 * 1024)
#define OUTPUTBUF_SIZE (44100 / 10)
#define OUTPUTBUF_SIZE_CROSSFADE (OUTPUTBUF_SIZE * 12 / 10)

#define MAX_HEADER 4096 // do not reduce as icy-meta max is 4080


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>

#include <lwip/inet.h>
#include <sys/socket.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "esp8266.h"
#include "espressif/esp_common.h"
#include "espressif/esp_misc.h"

// thread information
#define STREAM_THREAD_STACK_SIZE  64
#define DECODE_THREAD_STACK_SIZE 128
#define OUTPUT_THREAD_STACK_SIZE  64
#define IR_THREAD_STACK_SIZE      64
#define thread_t pthread_t;

#define usleep(x) sdk_os_delay_us(x)
#define sleep(x) vTaskDelay(x/portTICK_PERIOD_MS)
#define last_error() errno
#define ERROR_WOULDBLOCK EWOULDBLOCK
#define in_addr_t u32_t

#define event_event TaskHandle_t
#define event_handle u32_t
#define wake_create(e) e = xTaskGetCurrentTaskHandle()
#define wake_signal(e) xTaskNotifyGive(e)
#define wake_close(e) close(e)

typedef u32_t frames_t;
typedef int sockfd;

struct pollfd {
	int fd;
	short events;
	short revents;
};

#define POLLIN	0x0001
#define POLLOUT	0x0004
#define POLLHUP	0x0010

#define SL_LITTLE_ENDIAN (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

// primitivetype defs
typedef uint8_t  u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef uint64_t u64_t;
typedef int16_t  s16_t;
typedef int32_t  s32_t;
typedef int64_t  s64_t;


// mutex defines
#define mutex_type SemaphoreHandle_t
#define mutex_create(m) m = xSemaphoreCreateBinary()
#define mutex_create_p mutex_create
#define mutex_lock(m) xSemaphoreTake(m, portMAX_DELAY)
#define mutex_unlock(m) xSemaphoreGive(m)
#define mutex_destroy(m) vSemaphoreDelete(m)
#define thread_type TaskHandle_t

// utils.c (non logging)
typedef enum { EVENT_WAITING = 0, EVENT_TIMEOUT, EVENT_READ, EVENT_WAKE } event_type;

char *next_param(char *src, char c);
u32_t gettime_ms(void);
void get_mac(u8_t *mac);
void set_nonblock(sockfd s);
int connect_timeout(sockfd sock, const struct sockaddr *addr, socklen_t addrlen, int timeout);
void server_addr(char *server, in_addr_t *ip_ptr, unsigned *port_ptr);
void set_readwake_handles(event_handle handles[], sockfd s, event_event e);
event_type wait_readwake(event_handle handles[], int timeout);
void packN(u32_t *dest, u32_t val);
void packn(u16_t *dest, u16_t val);
u32_t unpackN(u32_t *src);
u16_t unpackn(u16_t *src);
int poll(struct pollfd *fds, unsigned long numfds, int timeout);
#define set_nosigpipe(s)


// buffer.c
struct buffer {
	u8_t *buf;
	u8_t *readp;
	u8_t *writep;
	u8_t *wrap;
	size_t size;
	size_t base_size;
	mutex_type mutex;
};

// _* called with mutex locked
unsigned _buf_used(struct buffer *buf);
unsigned _buf_space(struct buffer *buf);
unsigned _buf_cont_read(struct buffer *buf);
unsigned _buf_cont_write(struct buffer *buf);
void _buf_inc_readp(struct buffer *buf, unsigned by);
void _buf_inc_writep(struct buffer *buf, unsigned by);
void buf_flush(struct buffer *buf);
void buf_adjust(struct buffer *buf, size_t mod);
void _buf_resize(struct buffer *buf, size_t size);
void buf_init(struct buffer *buf, size_t size);
void buf_destroy(struct buffer *buf);


// slimproto.c
void slimproto(log_level level, char *server, u8_t mac[6], const char *name, const char *namefile, const char *modelname, int maxSampleRate);
void slimproto_stop(void);
void wake_controller(void);


// stream.c
typedef enum { STOPPED = 0, DISCONNECT, STREAMING_WAIT,
			   STREAMING_BUFFERING, STREAMING_FILE, STREAMING_HTTP, SEND_HEADERS, RECV_HEADERS } stream_state;
typedef enum { DISCONNECT_OK = 0, LOCAL_DISCONNECT = 1, REMOTE_DISCONNECT = 2, UNREACHABLE = 3, TIMEOUT = 4 } disconnect_code;

struct streamstate {
	stream_state state;
	disconnect_code disconnect;
	char *header;
	size_t header_len;
	bool sent_headers;
	bool cont_wait;
	u64_t bytes;
	unsigned threshold;
	u32_t meta_interval;
	u32_t meta_next;
	u32_t meta_left;
	bool  meta_send;
};

void stream_init(log_level level, unsigned stream_buf_size);
void stream_close(void);
void stream_file(const char *header, size_t header_len, unsigned threshold);
void stream_sock(u32_t ip, u16_t port, const char *header, size_t header_len, unsigned threshold, bool cont_wait);
bool stream_disconnect(void);


// decode.c
typedef enum { DECODE_STOPPED = 0, DECODE_READY, DECODE_RUNNING, DECODE_COMPLETE, DECODE_ERROR } decode_state;

struct decodestate {
	decode_state state;
	bool new_stream;
	mutex_type mutex;
#if PROCESS
	bool direct;
	bool process;
#endif
};

#if PROCESS
struct processstate {
	u8_t *inbuf, *outbuf;
	unsigned max_in_frames, max_out_frames;
	unsigned in_frames, out_frames;
	unsigned in_sample_rate, out_sample_rate;
	unsigned long total_in, total_out;
};
#endif

struct codec {
	char id;
	char *types;
	unsigned min_read_bytes;
	unsigned min_space;
	void (*open)(u8_t sample_size, u8_t sample_rate, u8_t channels, u8_t endianness);
	void (*cclose)(void);
	decode_state (*decode)(void);
};

void decode_init(log_level level, const char *include_codecs, const char *exclude_codecs);
void decode_close(void);
void decode_flush(void);
unsigned decode_newstream(unsigned sample_rate, unsigned supported_rates[]);
void codec_open(u8_t format, u8_t sample_size, u8_t sample_rate, u8_t channels, u8_t endianness);


#if PROCESS
// process.c
void process_samples(void);
void process_drain(void);
void process_flush(void);
unsigned process_newstream(bool *direct, unsigned raw_sample_rate, unsigned supported_rates[]);
void process_init(char *opt);
#endif


#if RESAMPLE
// resample.c
void resample_samples(struct processstate *process);
bool resample_drain(struct processstate *process);
bool resample_newstream(struct processstate *process, unsigned raw_sample_rate, unsigned supported_rates[]);
void resample_flush(void);
bool resample_init(char *opt);
#endif


// output.c output_alsa.c output_pa.c output_pack.c
typedef enum { OUTPUT_OFF = -1, OUTPUT_STOPPED = 0, OUTPUT_BUFFER, OUTPUT_RUNNING,
			   OUTPUT_PAUSE_FRAMES, OUTPUT_SKIP_FRAMES, OUTPUT_START_AT } output_state;

typedef enum { S32_LE, S24_LE, S24_3LE, S16_LE } output_format;

typedef enum { FADE_INACTIVE = 0, FADE_DUE, FADE_ACTIVE } fade_state;
typedef enum { FADE_UP = 1, FADE_DOWN, FADE_CROSS } fade_dir;
typedef enum { FADE_NONE = 0, FADE_CROSSFADE, FADE_IN, FADE_OUT, FADE_INOUT } fade_mode;

#define MAX_SUPPORTED_SAMPLERATES 16
#define TEST_RATES = { 384000, 352800, 192000, 176400, 96000, 88200, 48000, 44100, 32000, 24000, 22500, 16000, 12000, 11025, 8000, 0 }

struct outputstate {
	output_state state;
	output_format format;
	const char *device;
#if ALSA
	unsigned buffer;
	unsigned period;
#endif
	bool  track_started;
	int (* write_cb)(frames_t out_frames, bool silence, s32_t gainL, s32_t gainR, s32_t cross_gain_in, s32_t cross_gain_out, s32_t **cross_ptr);
	unsigned start_frames;
	unsigned frames_played;
	unsigned frames_played_dmp;// frames played at the point delay is measured
	unsigned current_sample_rate;
	unsigned supported_rates[MAX_SUPPORTED_SAMPLERATES]; // ordered largest first so [0] is max_rate
	unsigned default_sample_rate;
	bool error_opening;
	unsigned device_frames;
	u32_t updated;
	u32_t track_start_time;
	u32_t current_replay_gain;
	union {
		u32_t pause_frames;
		u32_t skip_frames;
		u32_t start_at;
	};
	unsigned next_sample_rate; // set in decode thread
	u8_t  *track_start;        // set in decode thread
	u32_t gainL;               // set by slimproto
	u32_t gainR;               // set by slimproto
	bool  invert;              // set by slimproto
	u32_t next_replay_gain;    // set by slimproto
	unsigned threshold;        // set by slimproto
	fade_state fade;
	u8_t *fade_start;
	u8_t *fade_end;
	fade_dir fade_dir;
	fade_mode fade_mode;       // set by slimproto
	unsigned fade_secs;        // set by slimproto
	unsigned rate_delay;
	bool delay_active;
	u32_t stop_time;
	u32_t idle_to;
};


void output_init_common(log_level level, const char *device, unsigned output_buf_size, unsigned rates[], unsigned idle);
void output_close_common(void);
void output_flush(void);
// _* called with mutex locked
frames_t _output_frames(frames_t avail);
void _checkfade(bool);


// output_alsa.c
#if ALSA
void list_devices(void);
void list_mixers(const char *output_device);
void set_volume(unsigned left, unsigned right);
bool test_open(const char *device, unsigned rates[]);
void output_init_alsa(log_level level, const char *device, unsigned output_buf_size, char *params, unsigned rates[], unsigned rate_delay, unsigned rt_priority, unsigned idle, char *mixer_device, char *volume_mixer, bool mixer_unmute, bool mixer_linear);
void output_close_alsa(void);
#endif

// output PCM5141
void set_volume(unsigned left, unsigned right);


// output_stdout.c
void output_init_stdout(log_level level, unsigned output_buf_size, char *params, unsigned rates[], unsigned rate_delay);
void output_close_stdout(void);


// output_pack.c
void _scale_and_pack_frames(void *outputptr, s32_t *inputptr, frames_t cnt, s32_t gainL, s32_t gainR, output_format format);
void _apply_cross(struct buffer *outputbuf, frames_t out_frames, s32_t cross_gain_in, s32_t cross_gain_out, s32_t **cross_ptr);
void _apply_gain(struct buffer *outputbuf, frames_t count, s32_t gainL, s32_t gainR);
s32_t gain(s32_t gain, s32_t sample);
s32_t to_gain(float f);


// output_vis.c
#define _vis_export(...)
#define vis_stop()


// codecs
#define MAX_CODECS 3

struct codec *register_flac(void);
struct codec *register_pcm(void);
struct codec *register_mad(void);


//gpio.c
#if SQUEEZE_GPIO
void relay( int state);
void relay_script(int state);
int gpio_pin;
bool gpio_active_low;
bool gpio_active;
char *power_script;
//  my amp state
int ampstate;
#endif

// ir.c
#if IR
struct irstate {
	mutex_type mutex;
	u32_t code;
	u32_t ts;
};

void ir_init(log_level level, char *lircrc);
void ir_close(void);
#endif
