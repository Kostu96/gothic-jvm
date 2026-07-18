#pragma once
#include <cstdint>

constexpr uint8_t op_nop = 0x00;
constexpr uint8_t op_aconst_null = 0x01;
constexpr uint8_t op_iconst_m1 = 0x02;
constexpr uint8_t op_iconst_0 = 0x03;
constexpr uint8_t op_iconst_1 = 0x04;
constexpr uint8_t op_iconst_2 = 0x05;
constexpr uint8_t op_iconst_3 = 0x06;
constexpr uint8_t op_iconst_4 = 0x07;
constexpr uint8_t op_iconst_5 = 0x08;
constexpr uint8_t op_lconst_0 = 0x09;
constexpr uint8_t op_lconst_1 = 0x0A;
constexpr uint8_t op_fconst_0 = 0x0B;
constexpr uint8_t op_fconst_1 = 0x0C;
constexpr uint8_t op_fconst_2 = 0x0D;
constexpr uint8_t op_dconst_0 = 0x0E;
constexpr uint8_t op_dconst_1 = 0x0F;
constexpr uint8_t op_bipush = 0x10;
constexpr uint8_t op_sipush = 0x11;
constexpr uint8_t op_ldc = 0x12;
constexpr uint8_t op_ldc_w = 0x13;
constexpr uint8_t op_ldc2_w = 0x14;
constexpr uint8_t op_iload = 0x15;
constexpr uint8_t op_lload = 0x16;

constexpr uint8_t op_aload = 0x19;
constexpr uint8_t op_iload_0 = 0x1A;
constexpr uint8_t op_iload_1 = 0x1B;
constexpr uint8_t op_iload_2 = 0x1C;
constexpr uint8_t op_iload_3 = 0x1D;

constexpr uint8_t op_lload_0 = 0x1E;
constexpr uint8_t op_lload_1 = 0x1F;
constexpr uint8_t op_lload_2 = 0x20;
constexpr uint8_t op_lload_3 = 0x21;

constexpr uint8_t op_aload_0 = 0x2A;
constexpr uint8_t op_aload_1 = 0x2B;
constexpr uint8_t op_aload_2 = 0x2C;
constexpr uint8_t op_aload_3 = 0x2D;
constexpr uint8_t op_iaload = 0x2E;

constexpr uint8_t op_aaload = 0x32;
constexpr uint8_t op_baload = 0x33;
constexpr uint8_t op_caload = 0x34;

constexpr uint8_t op_istore = 0x36;

constexpr uint8_t op_astore = 0x3A;
constexpr uint8_t op_istore_0 = 0x3B;
constexpr uint8_t op_istore_1 = 0x3C;
constexpr uint8_t op_istore_2 = 0x3D;
constexpr uint8_t op_istore_3 = 0x3E;

constexpr uint8_t op_astore_0 = 0x4B;
constexpr uint8_t op_astore_1 = 0x4C;
constexpr uint8_t op_astore_2 = 0x4D;
constexpr uint8_t op_astore_3 = 0x4E;
constexpr uint8_t op_iastore = 0x4F;

constexpr uint8_t op_aastore = 0x53;
constexpr uint8_t op_bastore = 0x54;
constexpr uint8_t op_castore = 0x55;
constexpr uint8_t op_sastore = 0x56;
constexpr uint8_t op_pop = 0x57;

constexpr uint8_t op_dup = 0x59;

constexpr uint8_t op_dup2 = 0x5C;

constexpr uint8_t op_iadd = 0x60;
constexpr uint8_t op_ladd = 0x61;

constexpr uint8_t op_isub = 0x64;

constexpr uint8_t op_imul = 0x68;

constexpr uint8_t op_idiv = 0x6C;

constexpr uint8_t op_irem = 0x70;

constexpr uint8_t op_ineg = 0x74;

constexpr uint8_t op_ishl = 0x78;

constexpr uint8_t op_ishr = 0x7A;

constexpr uint8_t op_iand = 0x7E;
constexpr uint8_t op_land = 0x7F;
constexpr uint8_t op_ior = 0x80;

constexpr uint8_t op_lxor = 0x83;
constexpr uint8_t op_iinc = 0x84;

constexpr uint8_t op_i2b = 0x91;

constexpr uint8_t op_i2s = 0x93;
constexpr uint8_t op_lcmp = 0x94;

constexpr uint8_t op_ifeq = 0x99;
constexpr uint8_t op_ifne = 0x9A;
constexpr uint8_t op_iflt = 0x9B;
constexpr uint8_t op_ifge = 0x9C;

constexpr uint8_t op_ifle = 0x9E;
constexpr uint8_t op_if_icmpeq = 0x9F;
constexpr uint8_t op_if_icmpne = 0xA0;
constexpr uint8_t op_if_icmplt = 0xA1;
constexpr uint8_t op_if_icmpge = 0xA2;
constexpr uint8_t op_if_icmpgt = 0xA3;
constexpr uint8_t op_if_icmple = 0xA4;

constexpr uint8_t op_goto = 0xA7;

constexpr uint8_t op_tableswitch = 0xAA;

constexpr uint8_t op_ireturn = 0xAC;

constexpr uint8_t op_areturn = 0xB0;
constexpr uint8_t op_return = 0xB1;
constexpr uint8_t op_getstatic = 0xB2;
constexpr uint8_t op_putstatic = 0xB3;
constexpr uint8_t op_getfield = 0xB4;
constexpr uint8_t op_putfield = 0xB5;
constexpr uint8_t op_invokevirtual = 0xB6;
constexpr uint8_t op_invokespecial = 0xB7;
constexpr uint8_t op_invokestatic = 0xB8;
constexpr uint8_t op_invokeinterface = 0xB9;

constexpr uint8_t op_new = 0xBB;
constexpr uint8_t op_newarray = 0xBC;
constexpr uint8_t op_anewarray = 0xBD;
constexpr uint8_t op_arraylength = 0xBE;
constexpr uint8_t op_athrow = 0xBF;
constexpr uint8_t op_checkcast = 0xC0;

constexpr uint8_t op_monitorenter = 0xC2;
constexpr uint8_t op_monitorexit = 0xC3;

constexpr uint8_t op_multianewarray = 0xC5;
constexpr uint8_t op_ifnull = 0xC6;
constexpr uint8_t op_ifnonnull = 0xC7;
