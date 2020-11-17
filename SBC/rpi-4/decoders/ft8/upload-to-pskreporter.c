#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

const char name[] = "report.pskreporter.info";
const char soft[] = "Radioberry FT8 Decoder 0.1";

int32_t read_int(char **pointer, int32_t *value)
{
  char *start = *pointer;
  *value = strtol(start, pointer, 10);
  return start != *pointer;
}

int32_t read_dbl(char **pointer, double *value)
{
  char *start = *pointer;
  *value = strtod(start, pointer);
  return start != *pointer;
}

int32_t read_time(char **pointer, struct tm *value)
{
  *pointer = strptime(*pointer, "%y%m%d %H%M%S", value);
  return *pointer != NULL;
}

void copy_char(char **pointer, const char *value)
{
  int8_t size = strlen(value);
  memcpy(*pointer, &size, 1);
  *pointer += 1;
  memcpy(*pointer, value, size);
  *pointer += size;
}

void copy_int1(char **pointer, int8_t value)
{
  memcpy(*pointer, &value, 1);
  *pointer += 1;
}

void copy_int2(char **pointer, int16_t value)
{
  value = htons(value);
  memcpy(*pointer, &value, 2);
  *pointer += 2;
}

void copy_int4(char **pointer, int32_t value)
{
  value = htonl(value);
  memcpy(*pointer, &value, 4);
  *pointer += 4;
}

int main(int argc, char *argv[])
{
  FILE *fp;
  int sock;
  struct hostent *host;
  struct sockaddr_in addr;
  struct tm tm;
  struct timespec ts;
  double sync, dt;
  int32_t snr, freq, counter, rc, padding, sequence, size;
  char buffer[512], line[64], call[16], grid[8], *src, *dst, *start;
  char header[] =
  {
    0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x00, 0x03, 0x00, 0x2C, 0x99, 0x92, 0x00, 0x04,
    0x00, 0x00,
    0x80, 0x02, 0xFF, 0xFF, 0x00, 0x00, 0x76, 0x8F,
    0x80, 0x04, 0xFF, 0xFF, 0x00, 0x00, 0x76, 0x8F,
    0x80, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x76, 0x8F,
    0x80, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x76, 0x8F,
    0x00, 0x00,

    0x00, 0x02, 0x00, 0x3C, 0x99, 0x93, 0x00, 0x07,
    0x80, 0x01, 0xFF, 0xFF, 0x00, 0x00, 0x76, 0x8F,
    0x80, 0x03, 0xFF, 0xFF, 0x00, 0x00, 0x76, 0x8F,
    0x80, 0x05, 0x00, 0x04, 0x00, 0x00, 0x76, 0x8F,
    0x80, 0x06, 0x00, 0x01, 0x00, 0x00, 0x76, 0x8F,
    0x80, 0x0A, 0xFF, 0xFF, 0x00, 0x00, 0x76, 0x8F,
    0x80, 0x0B, 0x00, 0x01, 0x00, 0x00, 0x76, 0x8F,
    0x00, 0x96, 0x00, 0x04
  };

  if(argc != 5)
  {
    fprintf(stderr, "Usage: upload-to-pskreporter call grid antenna file\n");
    return EXIT_FAILURE;
  }

  if(strlen(argv[1]) > 16)
  {
    fprintf(stderr, "Call sign is too long.\n");
    return EXIT_FAILURE;
  }

  if(strlen(argv[2]) > 6)
  {
    fprintf(stderr, "Grid locator is too long.\n");
    return EXIT_FAILURE;
  }

  if(strlen(argv[3]) > 64)
  {
    fprintf(stderr, "Antenna description is too long.\n");
    return EXIT_FAILURE;
  }

  if((fp = fopen(argv[4], "r")) == NULL)
  {
    fprintf(stderr, "Cannot open input file.\n");
    return EXIT_FAILURE;
  }

  if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    fprintf(stderr, "Cannot open socket.\n");
    return EXIT_FAILURE;
  }

  if((host = gethostbyname(name)) == NULL)
  {
    fprintf(stderr, "Cannot find remote host address.\n");
    return EXIT_FAILURE;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  memcpy(&addr.sin_addr.s_addr, host->h_addr, host->h_length);
  addr.sin_port = htons(4739);

  clock_gettime(CLOCK_REALTIME, &ts);
  srand(ts.tv_nsec / 1000);

  dst = header + 12;
  copy_int4(&dst, rand());

  memcpy(buffer, header, sizeof(header));

  start = buffer + sizeof(header);

  dst = start + 4;
  copy_char(&dst, argv[1]);
  copy_char(&dst, argv[2]);
  copy_char(&dst, soft);
  copy_char(&dst, argv[3]);

  size = dst - start;
  padding = (4 - size % 4) % 4;
  size += padding;
  memset(dst, 0, padding);

  dst = start;
  copy_int2(&dst, 0x9992);
  copy_int2(&dst, size);

  start += size;

  counter = 0;
  sequence = 0;
  dst = start + 4;
  for(;;)
  {
    src = fgets(line, 64, fp);

    if(src != NULL)
    {
      call[0] = 0;
      grid[0] = 0;
      rc = read_time(&src, &tm)
        && read_dbl(&src, &sync)
        && read_int(&src, &snr)
        && read_dbl(&src, &dt)
        && read_int(&src, &freq)
        && sscanf(src, "%13s %4s", call, grid);

      if(!rc) continue;

      copy_char(&dst, call);
      copy_char(&dst, grid);
      copy_int4(&dst, freq);
      copy_int1(&dst, snr);
      copy_char(&dst, "FT8");
      copy_int1(&dst, 1);
      copy_int4(&dst, timegm(&tm) + 15);

      ++counter;

      if(counter < 10) continue;
    }

    if(counter > 0)
    {
      size = dst - start;
      padding = (4 - size % 4) % 4;
      size += padding;
      memset(dst, 0, padding);

      dst = start;
      copy_int2(&dst, 0x9993);
      copy_int2(&dst, size);

      dst = buffer + 2;
      size += start - buffer;
      copy_int2(&dst, size);
      copy_int4(&dst, time(NULL));
      copy_int4(&dst, sequence);

		//fprintf(stderr, "send"); 
      int res = sendto(sock, buffer, size, 0, (struct sockaddr *)&addr, sizeof(addr));

		//fprintf(stderr, "sendresult: %d", res); 
      counter = 0;
      ++sequence;
      dst = start + 4;
    }

    if(src == NULL) break;
  }

  return EXIT_SUCCESS;
}
