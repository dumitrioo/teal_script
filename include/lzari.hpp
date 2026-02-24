#pragma once

#include "commondefs.hpp"
#include "hash/adler.hpp"

namespace scfx {

    DEFINE_RUNTIME_ERROR_CLASS_MSG(encoding_error, "data compression error")
    DEFINE_RUNTIME_ERROR_CLASS_MSG(decoding_error, "data decompression error")
    DEFINE_RUNTIME_ERROR_CLASS_MSG(wrong_or_corrupted_error, "data passed is corrupted or not an archive")

    using lzari_int_t = std::int32_t;
    using lzari_unsigned_long_int_t = std::uint64_t;
    using lzari_unsigned_int_t = std::uint32_t;

    class lzari {
        static lzari_int_t constexpr N = 4096;
        static lzari_int_t constexpr F	= 60;
        static lzari_int_t constexpr THRESHOLD = 2;
        static lzari_int_t constexpr NIL = N;
        static lzari_int_t constexpr M = 15;
        static lzari_unsigned_long_int_t constexpr Q1 = (1UL << M);
        static lzari_unsigned_long_int_t constexpr Q2 = (2 * Q1);
        static lzari_unsigned_long_int_t constexpr Q3 = (3 * Q1);
        static lzari_unsigned_long_int_t constexpr Q4 = (4 * Q1);
        static lzari_unsigned_long_int_t constexpr MAX_CUM = (Q1 - 1);
        static lzari_int_t constexpr N_CHAR = (256 - THRESHOLD + F);
        static lzari_int_t constexpr ENDOFFILE = -1;

    public:
        std::vector<std::uint8_t> encode(const std::vector<std::uint8_t> &in_container) {
            std::vector<std::uint8_t> out_container{};
            infile = &in_container;
            outfile = &out_container;
            reset();

            for(std::size_t j{0}; j < 4; j++) {
                outfile->push_back(0);
            }

            textsize = infile->size();

            std::uint8_t ibytes[sizeof(lzari_unsigned_long_int_t)];
            memcpy(ibytes, &textsize, sizeof(lzari_unsigned_long_int_t));
            for(std::size_t lkj = 0; lkj < sizeof(lzari_unsigned_long_int_t); lkj++) {
                outfile->push_back(ibytes[lkj]);
            }

            codesize += sizeof(lzari_unsigned_long_int_t);
            if(textsize == 0) {
                throw encoding_error{};
            }
            input_iterator = infile->begin();
            textsize = 0;
            start_model();
            init_tree();
            lzari_int_t s = 0;
            lzari_int_t r = N - F;
            for(lzari_int_t i = s; i < r; i++) {
                text_buf[i] = ' ';
            }
            lzari_int_t len;
            for(len = 0; len < F && (input_iterator != infile->end()); len++) {
                lzari_int_t c = get_c();
                text_buf[r + len] = c;
            }

            textsize = len;
            for(int i = 1; i <= F; i++) {
                insert_node(r - i);
            }
            insert_node(r);
            do
            {
                if(match_length > len) {
                    match_length = len;
                }
                if(match_length <= THRESHOLD) {
                    match_length = 1;
                    encode_char(text_buf[r]);
                } else {
                    encode_char(255 - THRESHOLD + match_length);
                    encode_position(match_position - 1);
                }

                lzari_int_t last_match_length{match_length};
                lzari_int_t i;
                for(i = 0; i < last_match_length && (input_iterator != infile->end()); i++) {
                    lzari_int_t c = get_c();
                    delete_node(s);
                    text_buf[s] = c;
                    if(s < F - 1) {
                        text_buf[s + N] = c;
                    }
                    s = (s + 1) & (N - 1);
                    r = (r + 1) & (N - 1);
                    insert_node(r);
                }

                textsize++;
                while(i++ < last_match_length) {
                    delete_node(s);
                    s = (s + 1) & (N - 1);
                    r = (r + 1) & (N - 1);
                    if(--len) {
                        insert_node(r);
                    }
                }
            } while(len > 0);
            encode_end();

            std::uint32_t asum{adler32(&out_container[4], out_container.size() - 4)};
            for(std::size_t i = 0; i < 4; ++i) {
                out_container[i] = (asum >> (i * 8)) & 0xff;
            }

            return out_container;
        }

        std::vector<std::uint8_t> decode(const std::vector<std::uint8_t> &in_container) {
            std::vector<std::uint8_t> out_container{};
            infile = &in_container;
            input_iterator = infile->begin();
            outfile = &out_container;
            reset();

            if(in_container.size() < sizeof(lzari_unsigned_long_int_t) + sizeof(lzari_unsigned_int_t) + 1) {
                throw decoding_error{};
            }

            std::uint32_t esum{adler32(&in_container[4], in_container.size() - 4)};
            std::uint32_t asum{};
            for(std::size_t i{0}; i < 4; ++i) {
                std::uint32_t b{(std::uint32_t)get_c()};
                asum |= (b & 0xff) << (i * 8);
            }
            if(asum != esum) {
                throw wrong_or_corrupted_error{};
            }

            unsigned char ibytes[sizeof(lzari_unsigned_long_int_t)];
            for(std::size_t lkj = 0; lkj < sizeof(lzari_unsigned_long_int_t) && (input_iterator != infile->end()); lkj++) {
                ibytes[lkj] = get_c();
            }

            memcpy(&textsize, ibytes, sizeof(lzari_unsigned_long_int_t));
            if(textsize == 0) {
                throw decoding_error{};
            }
            start_decode();
            start_model();
            for(lzari_int_t i = 0; i < N - F; i++) {
                text_buf[i] = ' ';
            }
            lzari_int_t r = N - F;
            for(lzari_unsigned_long_int_t count = 0; count < textsize;) {
                lzari_int_t c = decode_char();
                if(c < 256) {
                    put_c(c);
                    text_buf[r++] = c;
                    r &= (N - 1);
                    count++;
                } else {
                    lzari_int_t i = (r - decode_position() - 1) & (N - 1);
                    lzari_int_t j = c - 255 + THRESHOLD;
                    for(lzari_int_t k = 0; k < j; k++) {
                        c = text_buf[(i + k) & (N - 1)];
                        put_c(c);
                        text_buf[r++] = c;
                        r &= (N - 1);
                        count++;
                    }
                }
            }

            return out_container;
        }

        std::vector<std::uint8_t> decode(const void *pInput, size_t nInputSize) {
            std::vector<std::uint8_t> vecInput{reinterpret_cast<std::uint8_t const *>(pInput), reinterpret_cast<std::uint8_t const *>(pInput) + nInputSize};
            return decode(vecInput);
        }

        std::vector<std::uint8_t> encode(const void *pInput, size_t nInputSize) {
            std::vector<std::uint8_t> vecInput{reinterpret_cast<std::uint8_t const *>(pInput), reinterpret_cast<std::uint8_t const *>(pInput) + nInputSize};
            return encode(vecInput);
        }

        std::vector<std::uint8_t> encode(const std::string &in_container) {
            return encode(std::vector<std::uint8_t>{in_container.begin(), in_container.end()});
        }

    private:
        std::vector<std::uint8_t>::const_iterator input_iterator;
        std::vector<std::uint8_t> const *infile;
        std::vector<std::uint8_t> *outfile;
        lzari_unsigned_long_int_t  textsize, codesize, printcount;
        unsigned char  text_buf[N + F - 1];
        lzari_int_t		match_position, match_length, lson[N + 1], rson[N + 257], dad[N + 1];
        lzari_unsigned_long_int_t  low, high, value;
        lzari_int_t  shifts;
        lzari_int_t  char_to_sym[N_CHAR], sym_to_char[N_CHAR + 1];
        lzari_unsigned_int_t sym_freq[N_CHAR + 1];
        lzari_unsigned_int_t sym_cum[N_CHAR + 1];
        lzari_unsigned_int_t position_cum[N + 1];
        lzari_unsigned_int_t get_bit_buffer, get_bit_mask;
        lzari_unsigned_int_t put_bit_buffer, put_bit_mask;

        lzari_int_t put_c(lzari_int_t ch) {
            outfile->push_back(ch);
            return ch;
        }

        lzari_int_t get_c() {
            return (input_iterator == infile->end()) ? ENDOFFILE : *(input_iterator++);
        }

        void put_bit(lzari_int_t bit) {
            if(bit) {
                put_bit_buffer |= put_bit_mask;
            }
            if((put_bit_mask >>= 1) == 0) {
                put_c(put_bit_buffer);

                put_bit_buffer = 0;
                put_bit_mask = 128;
                codesize++;
            }
        }

        lzari_int_t get_bit(void) {
            if((get_bit_mask >>= 1) == 0) {
                get_bit_buffer = get_c();
                get_bit_mask = 128;
            }

            return ((get_bit_buffer & get_bit_mask) != 0);
        }

        void flush_bit_buffer(void) {
            lzari_int_t i;
            for(i = 0; i < 7; i++) {
                put_bit(0);
            }
        }

        void init_tree(void) {
            lzari_int_t i;
            for(i = N + 1; i <= N + 256; i++) {
                rson[i] = NIL;
            }
            for(i = 0; i < N; i++) {
                dad[i] = NIL;
            }
        }

        void insert_node(lzari_int_t r) {
            lzari_int_t i, p, cmp, temp;
            std::uint8_t *key;

            cmp = 1;
            key = &text_buf[r];
            p = N + 1 + key[0];
            rson[r] = lson[r] = NIL;
            match_length = 0;
            for(;;) {
                if(cmp >= 0) {
                    if(rson[p] != NIL) {
                        p = rson[p];
                    } else {
                        rson[p] = r;
                        dad[r] = p;
                        return;
                    }
                } else {
                    if(lson[p] != NIL) {
                        p = lson[p];
                    } else {
                        lson[p] = r;
                        dad[r] = p;
                        return;
                    }
                }

                for(i = 1; i < F; i++) {
                    if((cmp = key[i] - text_buf[p + i]) != 0) {
                        break;
                    }
                }
                if(i > THRESHOLD) {
                    if(i > match_length) {
                        match_position = (r - p) & (N - 1);
                        if((match_length = i) >= F) {
                            break;
                        }
                    } else if(i == match_length) {
                        if((temp = (r - p) & (N - 1)) < match_position) {
                            match_position = temp;
                        }
                    }
                }
            }

            dad[r] = dad[p];
            lson[r] = lson[p];
            rson[r] = rson[p];
            dad[lson[p]] = r;
            dad[rson[p]] = r;
            if(rson[dad[p]] == p) {
                rson[dad[p]] = r;
            } else {
                lson[dad[p]] = r;
            }
            dad[p] = NIL;
        }

        void delete_node(lzari_int_t p) {
            lzari_int_t q;
            if(dad[p] == NIL) {
                return;
            }
            if(rson[p] == NIL) {
                q = lson[p];
            } else if(lson[p] == NIL) {
                q = rson[p];
            } else {
                q = lson[p];
                if(rson[q] != NIL) {
                    do
                    {
                        q = rson[q];
                    } while(rson[q] != NIL);
                    rson[dad[q]] = lson[q];
                    dad[lson[q]] = dad[q];
                    lson[q] = lson[p];
                    dad[lson[p]] = q;
                }

                rson[q] = rson[p];
                dad[rson[p]] = q;
            }

            dad[q] = dad[p];
            if(rson[dad[p]] == p) {
                rson[dad[p]] = q;
            } else {
                lson[dad[p]] = q;
            }
            dad[p] = NIL;
        }

        void start_model(void) {
            lzari_int_t ch, sym, i;
            sym_cum[N_CHAR] = 0;
            for(sym = N_CHAR; sym >= 1; sym--) {
                ch = sym - 1;
                char_to_sym[ch] = sym;
                sym_to_char[sym] = ch;
                sym_freq[sym] = 1;
                sym_cum[sym - 1] = sym_cum[sym] + sym_freq[sym];
            }

            sym_freq[0] = 0;
            position_cum[N] = 0;
            for(i = N; i >= 1; i--) {
                position_cum[i - 1] = position_cum[i] + 10000 / (i + 200);
            }
        }

        void update_model(lzari_int_t sym) {
            lzari_int_t i, c, ch_i, ch_sym;

            if(sym_cum[0] >= MAX_CUM) {
                c = 0;
                for(i = N_CHAR; i > 0; i--) {
                    sym_cum[i] = c;
                    c += (sym_freq[i] = (sym_freq[i] + 1) >> 1);
                }

                sym_cum[0] = c;
            }

            for(i = sym; sym_freq[i] == sym_freq[i - 1]; i--) {}
            if(i < sym) {
                ch_i = sym_to_char[i];
                ch_sym = sym_to_char[sym];
                sym_to_char[i] = ch_sym;
                sym_to_char[sym] = ch_i;
                char_to_sym[ch_i] = sym;
                char_to_sym[ch_sym] = i;
            }

            sym_freq[i]++;
            while(--i >= 0) {
                sym_cum[i]++;
            }
        }

        void output(lzari_int_t bit) {
            put_bit(bit);
            for(; shifts > 0; shifts--) {
                put_bit(!bit);
            }
        }

        void encode_char(lzari_int_t ch) {
            lzari_int_t					sym;
            lzari_unsigned_long_int_t	range;

            sym = char_to_sym[ch];
            range = high - low;
            high = low + (range * sym_cum[sym - 1]) / sym_cum[0];
            low += (range * sym_cum[sym]) / sym_cum[0];
            for(;;) {
                if(high <= Q2) {
                    output(0);
                } else if(low >= Q2) {
                    output(1);
                    low -= Q2;
                    high -= Q2;
                } else if(low >= Q1 && high <= Q3) {
                    shifts++;
                    low -= Q1;
                    high -= Q1;
                } else {
                    break;
                }
                low += low;
                high += high;
            }

            update_model(sym);
        }

        void encode_position(lzari_int_t position) {
            lzari_unsigned_long_int_t	range;

            range = high - low;
            high = low + (range * position_cum[position]) / position_cum[0];
            low += (range * position_cum[position + 1]) / position_cum[0];
            for(;;) {
                if(high <= Q2) {
                    output(0);
                } else if(low >= Q2) {
                    output(1);
                    low -= Q2;
                    high -= Q2;
                } else if(low >= Q1 && high <= Q3) {
                    shifts++;
                    low -= Q1;
                    high -= Q1;
                } else {
                    break;
                }
                low += low;
                high += high;
            }
        }

        void encode_end(void) {
            shifts++;
            if(low < Q1) {
                output(0);
            } else {
                output(1);
            }
            flush_bit_buffer();
        }

        lzari_int_t binary_search_sym(lzari_unsigned_int_t x) {
            lzari_int_t i, j, k;
            i = 1;
            j = N_CHAR;
            while(i < j) {
                k = (i + j) / 2;
                if(sym_cum[k] > x) {
                    i = k + 1;
                } else {
                    j = k;
                }
            }

            return i;
        }

        lzari_int_t binary_search_pos(lzari_unsigned_int_t x) {
            lzari_int_t i, j, k;
            i = 1;
            j = N;
            while(i < j) {
                k = (i + j) / 2;
                if(position_cum[k] > x) {
                    i = k + 1;
                } else {
                    j = k;
                }
            }

            return i - 1;
        }

        void start_decode(void) {
            lzari_int_t i;
            for(i = 0; i < M + 2; i++) {
                value = 2 * value + get_bit();
            }
        }

        lzari_int_t decode_char(void) {
            lzari_int_t					sym, ch;
            lzari_unsigned_long_int_t	range;
            range = high - low;
            sym = binary_search_sym((lzari_unsigned_int_t) (((value - low + 1) * sym_cum[0] - 1) / range));
            high = low + (range * sym_cum[sym - 1]) / sym_cum[0];
            low += (range * sym_cum[sym]) / sym_cum[0];
            for(;;) {
                if(low >= Q2) {
                    value -= Q2;
                    low -= Q2;
                    high -= Q2;
                } else if(low >= Q1 && high <= Q3) {
                    value -= Q1;
                    low -= Q1;
                    high -= Q1;
                } else if(high > Q2) {
                    break;
                }
                low += low;
                high += high;
                value = 2 * value + get_bit();
            }

            ch = sym_to_char[sym];
            update_model(sym);
            return ch;
        }

        lzari_int_t decode_position(void) {
            lzari_int_t					position;
            lzari_unsigned_long_int_t	range;
            range = high - low;
            position = binary_search_pos((lzari_unsigned_int_t) (((value - low + 1) * position_cum[0] - 1) / range));
            high = low + (range * position_cum[position]) / position_cum[0];
            low += (range * position_cum[position + 1]) / position_cum[0];
            for(;;) {
                if(low >= Q2) {
                    value -= Q2;
                    low -= Q2;
                    high -= Q2;
                } else if(low >= Q1 && high <= Q3) {
                    value -= Q1;
                    low -= Q1;
                    high -= Q1;
                } else if(high > Q2) {
                    break;
                }
                low += low;
                high += high;
                value = 2 * value + get_bit();
            }

            return position;
        }

        void reset() {
            textsize = 0;
            codesize = 0;
            printcount = 0;
            low = 0;
            high = Q4;
            value = 0;
            shifts = 0;
            get_bit_buffer = 0;
            get_bit_mask = 0;
            put_bit_buffer = 0;
            put_bit_mask = 128;
            outfile->clear();
            input_iterator = infile->begin();
        }
    };

}
