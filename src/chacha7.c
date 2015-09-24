/* $OpenBSD: chacha7.c,v 1.5 2014/06/24 18:12:09 jsing Exp $ */
/*
 * Copyright (c) 2014 Joel Sing <jsing@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
 
 //This is a patched version, which only performs seven rounds of chacha.


#include <openssl/chacha.h>

#include "chacha-merged7.ipp"

void ChaCha7(ChaCha_ctx *ctx, unsigned char *out, const unsigned char *in, size_t len)
{
	unsigned char *k;
	int i, l;

	/* Consume remaining keystream, if any exists. */
	if (ctx->unused > 0) {
		k = ctx->ks + 64 - ctx->unused;
		l = (len > ctx->unused) ? ctx->unused : len;
		for (i = 0; i < l; i++)
			*(out++) = *(in++) ^ *(k++);
		ctx->unused -= l;
		len -= l;
	}

	chacha_encrypt_bytes7((chacha_ctx *)ctx, in, out, (uint32_t)len);
}