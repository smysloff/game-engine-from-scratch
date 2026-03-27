
// file: examples/chat/srv.c

#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define HOST "0.0.0.0"
#define PORT "6969"

typedef struct
{
  int fd;
  struct sockaddr_storage addr; // IPv4 & IPv6
  socklen_t len;
} socket_t;

int
create_server(socket_t *server, const char *address, const char *port, int ai_family)
{
  struct addrinfo hints;
  struct addrinfo *addr, *addrs;
  int opt;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = ai_family;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(address, port, &hints, &addrs) != 0)
  {
    perror("getaddrinfo()");
    return -1;
  }

  for (addr = addrs; addr; addr = addr->ai_next)
  {
    if ((server->fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) == -1)
      continue;

    opt = 1;
    if (setsockopt(server->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
      close(server->fd);
      continue;
    }

    if (bind(server->fd, addr->ai_addr, addr->ai_addrlen) == 0)
    {
      memcpy(&server->addr, addr->ai_addr, addr->ai_addrlen);
      server->len = addr->ai_addrlen;
      break;
    }

    close(server->fd);
  }

  freeaddrinfo(addrs);

  if (addr == NULL) // @todo test!
  {
    fprintf(stderr, "Could not bind\n");
    return -1;
  }

  if (listen(server->fd, SOMAXCONN) == -1)
  {
    perror("listen()");
    close(server->fd);
    return -1;
  }

  return server->fd;
}

Для получения IP-адреса и порта клиента вам нужно использовать функции inet_ntop() (или inet_ntoa() для IPv4) и ntohs(). Вот исправленный код:
Полная версия с получением IP и порта
c

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

typedef struct
{
    int fd;
    struct sockaddr_storage addr;  // Используем sockaddr_storage для поддержки IPv4/IPv6
    socklen_t len;
} client_t;

void print_client_info(client_t *client)
{
    char ip_str[INET6_ADDRSTRLEN];
    int port;

    // Определяем семейство адресов
    if (client->addr.ss_family == AF_INET)
    {
        // IPv4
        struct sockaddr_in *ipv4 = (struct sockaddr_in*)&client->addr;
        inet_ntop(AF_INET, &ipv4->sin_addr, ip_str, sizeof(ip_str));
        port = ntohs(ipv4->sin_port);
    }
    else if (client->addr.ss_family == AF_INET6)
    {
        // IPv6
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6*)&client->addr;
        inet_ntop(AF_INET6, &ipv6->sin6_addr, ip_str, sizeof(ip_str));
        port = ntohs(ipv6->sin6_port);
    }
    else
    {
        strcpy(ip_str, "Unknown");
        port = 0;
    }

    printf("SERVER: client %s:%d has joined\n", ip_str, port);
}

int get_client(int server, client_t *client)
{
    if (!client)
        return -1;

    client->len = sizeof(client->addr);
    client->fd = accept(server, (struct sockaddr*)&client->addr, &client->len);

    return client->fd;
}

int main(void)
{
    int server;
    client_t client;

    if ((server = create_server(HOST, PORT, AF_INET)) == -1)
        return EXIT_FAILURE;

    printf("SERVER: start listen on port %d\n", PORT);

    if (get_client(server, &client) == -1)
    {
        perror("get_client()");
        return EXIT_FAILURE;
    }

    print_client_info(&client);  // Выводим информацию о клиенте

    // Работа с клиентом...

    close(client.fd);
    close(server);
    return EXIT_SUCCESS;
}

Альтернативный вариант с использованием getnameinfo()

Более универсальный способ, который автоматически обрабатывает IPv4 и IPv6:
c

void print_client_info_v2(client_t *client)
{
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

    int ret = getnameinfo((struct sockaddr*)&client->addr, client->len,
                          host, sizeof(host),
                          service, sizeof(service),
                          NI_NUMERICHOST | NI_NUMERICSERV);

    if (ret == 0)
    {
        printf("SERVER: client %s:%s has joined\n", host, service);
    }
    else
    {
        fprintf(stderr, "getnameinfo(): %s\n", gai_strerror(ret));
    }
}

Ключевые моменты
1. Использование sockaddr_storage
c

struct sockaddr_storage addr;  // вместо struct sockaddr addr

Это важно, потому что struct sockaddr может быть недостаточно большим для IPv6 адресов.
2. Получение IP адреса

    IPv4: inet_ntop(AF_INET, &ipv4->sin_addr, ip_str, sizeof(ip_str))

    IPv6: inet_ntop(AF_INET6, &ipv6->sin6_addr, ip_str, sizeof(ip_str))

3. Получение порта

    Всегда используйте ntohs() для преобразования из сетевого порядка байт в хостовый

    Для IPv4: ntohs(ipv4->sin_port)

    Для IPv6: ntohs(ipv6->sin6_port)

4. Размер буфера для IP

    Для IPv4 достаточно INET_ADDRSTRLEN (16 байт)

    Для IPv6 используйте INET6_ADDRSTRLEN (46 байт)

    Для универсальности используйте INET6_ADDRSTRLEN

Пример вывода
text

SERVER: start listen on port 8080
SERVER: client 192.168.1.100:54321 has joined

Дополнительные функции для работы с клиентами
c

// Получить только IP (как строку)
char* get_client_ip(client_t *client, char *buffer, size_t size)
{
    if (client->addr.ss_family == AF_INET)
    {
        struct sockaddr_in *ipv4 = (struct sockaddr_in*)&client->addr;
        inet_ntop(AF_INET, &ipv4->sin_addr, buffer, size);
    }
    else if (client->addr.ss_family == AF_INET6)
    {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6*)&client->addr;
        inet_ntop(AF_INET6, &ipv6->sin6_addr, buffer, size);
    }
    else
    {
        strncpy(buffer, "Unknown", size);
    }

    return buffer;
}

// Получить порт клиента
int get_client_port(client_t *client)
{
    if (client->addr.ss_family == AF_INET)
    {
        struct sockaddr_in *ipv4 = (struct sockaddr_in*)&client->addr;
        return ntohs(ipv4->sin_port);
    }
    else if (client->addr.ss_family == AF_INET6)
    {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6*)&client->addr;
        return ntohs(ipv6->sin6_port);
    }

    return -1;
}

Важное замечание

В вашем исходном коде была проблема в строке:
c

printf("SERVER: client %s:%s has joined\n");  // Нет аргументов!

Я исправил это в примерах выше, добавив функцию print_client_info(), которая правильно форматирует и выводит IP и порт клиента.
А если переделать так?

typedef struct
{
  int fd;
  struct sockaddr_storage addr; // IPv4 & IPv6
  socklen_t len;
} socket_t;

int
create_server(socket_t *server, const char *address, int port, int ai_family)
{
  int opt;

  opt = 1;

  memset(&server->addr, 0, sizeof(server->addr));
  server->addr.ss_family = ai_family;
  server->addr.sin_addr.s_addr = htonl(HOST); // ?
  server->addr.sin_port = htons(PORT);        // ?

  if ((server->fd = socket(ai_family, SOCK_STREAM, IPPROTO_TCP)) == -1)
    return -1;

  if (setsockopt(server->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
  {
    perror("setsockopt()");
    close(server->fd);
    return -1;
  }

  if (bind(server->fd, (struct sockaddr *) &server->addr, sizeof(server->addr)) == -1)
  {
    perror("bind()");
    close(server->fd);
    return -1;
  }

  if (listen(server->fd, SOMAXCONN) == -1)
  {
    perror("listen()");
    close(server->fd);
    return -1;
  }

  return server->fd;
}

Ваша попытка имеет несколько проблем. struct sockaddr_storage - это универсальный контейнер, но вы не можете напрямую обращаться к полям sin_addr и sin_port, так как они относятся только к struct sockaddr_in. Давайте исправим:
Исправленная версия
c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

typedef struct
{
    int fd;
    struct sockaddr_storage addr;
    socklen_t len;
} socket_t;

// Вспомогательная функция для установки адреса
int set_address(struct sockaddr_storage *addr, const char *address, int port, int ai_family)
{
    memset(addr, 0, sizeof(*addr));
    addr->ss_family = ai_family;

    if (ai_family == AF_INET)
    {
        struct sockaddr_in *ipv4 = (struct sockaddr_in*)addr;
        ipv4->sin_family = AF_INET;
        ipv4->sin_port = htons(port);

        if (address == NULL || strcmp(address, "0.0.0.0") == 0 || strcmp(address, "*") == 0)
        {
            ipv4->sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else
        {
            if (inet_pton(AF_INET, address, &ipv4->sin_addr) != 1)
            {
                fprintf(stderr, "Invalid IPv4 address: %s\n", address);
                return -1;
            }
        }
    }
    else if (ai_family == AF_INET6)
    {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6*)addr;
        ipv6->sin6_family = AF_INET6;
        ipv6->sin6_port = htons(port);

        if (address == NULL || strcmp(address, "::") == 0 || strcmp(address, "*") == 0)
        {
            ipv6->sin6_addr = in6addr_any;
        }
        else
        {
            if (inet_pton(AF_INET6, address, &ipv6->sin6_addr) != 1)
            {
                fprintf(stderr, "Invalid IPv6 address: %s\n", address);
                return -1;
            }
        }
    }
    else
    {
        fprintf(stderr, "Unsupported address family: %d\n", ai_family);
        return -1;
    }

    return 0;
}

int create_server(socket_t *server, const char *address, int port, int ai_family)
{
    int opt = 1;

    if (!server)
    {
        fprintf(stderr, "create_server(): server is NULL\n");
        return -1;
    }

    // Устанавливаем адрес
    if (set_address(&server->addr, address, port, ai_family) == -1)
        return -1;

    server->len = sizeof(server->addr);

    // Создаем сокет
    if ((server->fd = socket(ai_family, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        perror("socket()");
        return -1;
    }

    // Устанавливаем SO_REUSEADDR
    if (setsockopt(server->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt(SO_REUSEADDR)");
        close(server->fd);
        return -1;
    }

    // Привязываем сокет
    if (bind(server->fd, (struct sockaddr*)&server->addr, server->len) == -1)
    {
        perror("bind()");
        close(server->fd);
        return -1;
    }

    // Начинаем слушать
    if (listen(server->fd, SOMAXCONN) == -1)
    {
        perror("listen()");
        close(server->fd);
        return -1;
    }

    return server->fd;
}

// Функция для получения информации о сервере
void print_server_info(socket_t *server)
{
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

    if (getnameinfo((struct sockaddr*)&server->addr, server->len,
                    host, sizeof(host),
                    service, sizeof(service),
                    NI_NUMERICHOST | NI_NUMERICSERV) == 0)
    {
        printf("Server listening on %s:%s\n", host, service);
    }
}

// Функция для принятия клиента
int accept_client(socket_t *server, socket_t *client)
{
    if (!server || !client)
        return -1;

    client->len = sizeof(client->addr);
    client->fd = accept(server->fd, (struct sockaddr *) &client->addr, &client->len);

    return client->fd;
}

void
get_socket_info(socket_t *sock)
{
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    if (getnameinfo((struct sockaddr*)&client->addr, client->len,
                    host, sizeof(host),
                    service, sizeof(service),
                    NI_NUMERICHOST | NI_NUMERICSERV) == 0)
    {
        printf("Client connected: %s:%s\n", host, service);
    }
}

int
get_client(socket_t *server, socket_t *client)
{
  if (!server || !client)
    return -1;

  client->len = sizeof(client->addr);
  client->fd = accept(server->fd, (struct sockaddr *) &client->addr, &client->len);

  return client->fd;
}


int
main(void)
{
  socket_t server;
  socket_t client;

  if (create_server(&server, HOST, PORT, AF_INET) == -1)
    return EXIT_FAILURE;

  printf("SERVER: start listen on port %d\n", );

  if (get_client(&server, &client) == -1)
  {
    perror("get_client()");
    return EXIT_FAILURE;
  }

  printf("SERVER: client %s:%s has joined\n");

  //for (bool loop = true; loop; )
  //{

  //}

  close(server);
  return EXIT_SUCCESS;
}
