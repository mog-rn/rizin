// SPDX-FileCopyrightText: 2018-2019 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

#include <rz_asm.h>
#include <rz_lib.h>
#include "cs_helper.h"

CAPSTONE_DEFINE_PLUGIN_FUNCTIONS(m680x);

static cs_mode m680x_mode(const char *str) {
	if (!str) {
		return CS_MODE_M680X_6800;
	}
	// replace this with the asm.features?
	if (strstr(str, "6800")) {
		return CS_MODE_M680X_6800;
	}
	if (strstr(str, "6801")) {
		return CS_MODE_M680X_6801;
	} else if (strstr(str, "6805")) {
		return CS_MODE_M680X_6805;
	} else if (strstr(str, "6808")) {
		return CS_MODE_M680X_6808;
	} else if (strstr(str, "6809")) {
		return CS_MODE_M680X_6809;
	} else if (strstr(str, "6811")) {
		return CS_MODE_M680X_6811;
	} else if (strstr(str, "cpu12")) {
		return CS_MODE_M680X_CPU12;
	} else if (strstr(str, "6301")) {
		return CS_MODE_M680X_6301;
	}
	if (strstr(str, "6309")) {
		return CS_MODE_M680X_6309;
	}
	if (strstr(str, "hcs08")) {
		return CS_MODE_M680X_HCS08;
	}
	return CS_MODE_M680X_6800;
}

static int m680x_disassemble(RzAsm *a, RzAsmOp *op, const ut8 *buf, int len) {
	CapstoneContext *ctx = (CapstoneContext *)a->plugin_data;

	int n, ret;
	ut64 off = a->pc;
	cs_mode mode;
	cs_insn *insn = NULL;
	mode = m680x_mode(a->cpu);
	op->size = 0;

	if (ctx->omode != mode) {
		cs_close(&ctx->handle);
		ctx->omode = -1;
	}
	if (!ctx->handle) {
		ret = cs_open(CS_ARCH_M680X, mode, &ctx->handle);
		if (ret) {
			return -1;
		}
		ctx->omode = mode;
		cs_option(ctx->handle, CS_OPT_DETAIL, CS_OPT_OFF);
	}

	n = cs_disasm(ctx->handle, (const ut8 *)buf, len, off, 1, &insn);
	if (n > 0) {
		if (insn->size > 0) {
			op->size = insn->size;
			rz_asm_op_setf_asm(op, "%s%s%s",
				insn->mnemonic, insn->op_str[0] ? " " : "",
				insn->op_str);
			char *ptrstr = strstr(rz_strbuf_get(&op->buf_asm), "ptr ");
			if (ptrstr) {
				memmove(ptrstr, ptrstr + 4, strlen(ptrstr + 4) + 1);
			}
		}
		cs_free(insn, n);
	}
	return op->size;
}

RzAsmPlugin rz_asm_plugin_m680x_cs = {
	.name = "m680x",
	.cpus = "6800,6801,6805,6808,6809,6811,cpu12,6301,6309,hcs08",
	.desc = "Capstone M680X Disassembler",
	.license = "BSD",
	.arch = "m680x",
	.bits = 8 | 32,
	.endian = RZ_SYS_ENDIAN_LITTLE,
	.init = m680x_init,
	.fini = m680x_fini,
	.disassemble = &m680x_disassemble,
	.mnemonics = m680x_mnemonics,
};

#ifndef RZ_PLUGIN_INCORE
RZ_API RzLibStruct rizin_plugin = {
	.type = RZ_LIB_TYPE_ASM,
	.data = &rz_asm_plugin_m680x_cs,
	.version = RZ_VERSION
};
#endif
