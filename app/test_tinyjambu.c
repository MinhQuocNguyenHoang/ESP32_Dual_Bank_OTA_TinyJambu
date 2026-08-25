#include <stdio.h>
#include <stdint.h>
#include <string.h>

static inline uint32_t le_load32(const uint8_t *p) {
    return ((uint32_t)p[0]) | (((uint32_t)p[1]) << 8) | (((uint32_t)p[2]) << 16) | (((uint32_t)p[3]) << 24);
}

static inline void le_store32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static void state_update(uint32_t s[4], const uint32_t k[4], unsigned int steps) {
    uint32_t t1, t2, t3, t4;
    for (unsigned int i = 0; i < steps; i += 32) {
        uint32_t kword = k[(i / 32) % 4];
        t1 = (s[1] >> 15) | (s[2] << 17);
        t2 = (s[2] >> 6)  | (s[3] << 26);
        t3 = (s[2] >> 21) | (s[3] << 11);
        t4 = (s[2] >> 27) | (s[3] << 5);
        s[0] ^= t1 ^ (t2 & t3) ^ t4 ^ kword;
        uint32_t temp = s[0];
        s[0] = s[1];
        s[1] = s[2];
        s[2] = s[3];
        s[3] = temp;
    }
}

int decrypt_test(const unsigned char *c, size_t clen, const unsigned char *npub, const unsigned char *k, uint8_t dom_n, uint8_t dom_m) {
    uint32_t s[4] = {0};
    uint32_t key_w[4];
    for (int i = 0; i < 4; i++) key_w[i] = ~le_load32(k + i * 4);

    s[1] ^= dom_n;
    state_update(s, key_w, 1024);
    s[3] ^= le_load32(npub);
    s[1] ^= dom_n;
    state_update(s, key_w, 384);
    s[3] ^= le_load32(npub + 4);
    s[1] ^= dom_n;
    state_update(s, key_w, 384);
    s[3] ^= le_load32(npub + 8);

    size_t msg_len = clen - 8;
    size_t msg_idx = 0;
    unsigned char m[64] = {0};
    while (msg_idx + 4 <= msg_len) {
        s[1] ^= dom_m;
        state_update(s, key_w, 1024);
        uint32_t c_word = le_load32(c + msg_idx);
        uint32_t p_word = c_word ^ s[2];
        le_store32(m + msg_idx, p_word);
        s[3] ^= p_word;
        msg_idx += 4;
    }
    if (msg_idx < msg_len) {
        s[1] ^= dom_m;
        state_update(s, key_w, 1024);
        size_t rem = msg_len - msg_idx;
        uint32_t s2_word = s[2];
        uint8_t p_bytes[4] = {0};
        for (size_t i = 0; i < rem; i++) {
            uint8_t p_byte = c[msg_idx + i] ^ ((uint8_t *)&s2_word)[i];
            m[msg_idx + i] = p_byte;
            p_bytes[i] = p_byte;
        }
        s[3] ^= le_load32(p_bytes);
        s[1] ^= (uint32_t)rem;
    }
    m[msg_len] = '\0';
    if (strstr((char*)m, "GLUCOSE:") != NULL) {
        printf("SUCCESS! Found match with dom_n=%d, dom_m=%d -> '%s'\n", dom_n, dom_m, m);
        return 1;
    }
    return 0;
}

int main() {
    const unsigned char secret_key[16] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    const unsigned char nonce[12] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B};

    const char *user_hex = "9A8CF6822523797D0B460500E39BAAF0D566EC4A3605";
    unsigned char user_c[32];
    size_t user_clen = strlen(user_hex) / 2;
    for (size_t i = 0; i < user_clen; i++) {
        sscanf(user_hex + i * 2, "%02hhx", &user_c[i]);
    }

    uint8_t domains[] = {1, 3, 5, 7, 0x10, 0x30, 0x50, 0x70};
    for (int dn = 0; dn < 8; dn++) {
        for (int dm = 0; dm < 8; dm++) {
            decrypt_test(user_c, user_clen, nonce, secret_key, domains[dn], domains[dm]);
            decrypt_test(user_c, user_clen, secret_key, nonce, domains[dn], domains[dm]); // key/nonce swap
        }
    }
    return 0;
}
