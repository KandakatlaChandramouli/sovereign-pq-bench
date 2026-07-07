/*
 * CBOM TLS FINGERPRINT SCANNER
 * Actively connects to targets, performs a TLS handshake,
 * and extracts the cipher suite + TLS version.
 * Compile: gcc -O2 -Wall cbom_tls_fingerprint.c -o cbom_tls_fingerprint -lssl -lcrypto
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define TIMEOUT_SEC 2

typedef struct {
    char ip[64];
    int port;
    char tls_version[32];
    char cipher[64];
    int bits;
    int risk;
    char pq_recommendation[32];
} TlsFingerprint;

static int tcp_connect_timeout(const char *ip, int port, int timeout_sec) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;
    struct timeval tv = {timeout_sec, 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }
    return sock;
}

static void fingerprint_tls(int sock, TlsFingerprint *fp) {
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    SSL_CTX_set_security_level(ctx, 0); // allow old TLS
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);
    SSL_set_connect_state(ssl);

    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        strcpy(fp->tls_version, "Handshake failed");
        strcpy(fp->cipher, "Unknown");
        fp->bits = 0;
        fp->risk = 80;
    } else {
        strcpy(fp->tls_version, SSL_get_version(ssl));
        const char *cipher = SSL_CIPHER_get_name(SSL_get_current_cipher(ssl));
        strncpy(fp->cipher, cipher, sizeof(fp->cipher)-1);
        fp->bits = SSL_CIPHER_get_bits(SSL_get_current_cipher(ssl), NULL);

        // Risk assessment
        if (strstr(fp->tls_version, "1.0") || strstr(fp->tls_version, "1.1")) {
            fp->risk = 95;
            strcpy(fp->pq_recommendation, "ML-KEM-768 + ML-DSA-44");
        } else if (strstr(fp->cipher, "RSA") || strstr(fp->cipher, "ECDSA")) {
            fp->risk = 85;
            strcpy(fp->pq_recommendation, "ML-DSA-44");
        } else {
            fp->risk = 40;
            strcpy(fp->pq_recommendation, "Monitor");
        }
    }

    SSL_free(ssl);
    SSL_CTX_free(ctx);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        printf("Example: %s 192.168.1.1 443\n", argv[0]);
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║  CBOM TLS Fingerprint Scanner                   ║\n");
    printf("╚══════════════════════════════════════════════════╝\n\n");

    TlsFingerprint fp = {0};
    strncpy(fp.ip, ip, sizeof(fp.ip)-1);
    fp.port = port;

    int sock = tcp_connect_timeout(ip, port, TIMEOUT_SEC);
    if (sock < 0) {
        printf("[CBOM] Could not connect to %s:%d\n", ip, port);
        return 1;
    }

    fingerprint_tls(sock, &fp);
    close(sock);

    printf("IP:            %s\n", fp.ip);
    printf("Port:          %d\n", fp.port);
    printf("TLS Version:   %s\n", fp.tls_version);
    printf("Cipher Suite:  %s\n", fp.cipher);
    printf("Key Bits:      %d\n", fp.bits);
    printf("Risk Score:    %d%%\n", fp.risk);
    printf("PQC Migration: %s\n", fp.pq_recommendation);

    return 0;
}
