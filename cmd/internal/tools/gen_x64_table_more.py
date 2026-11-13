# The rest of AVX-512 and the later VEX extensions for gen_x64_table.py, which
# includes this file (it defines ln() and the list L): the AVX-512 F/CD/BW/DQ
# forms beyond gen_x64_table_vec.py's families, VBMI2, BITALG, VNNI, IFMA,
# VP2INTERSECT, BF16, FP16 (the EVEX maps 5 and 6), ER, PF, 4FMAPS and 4VNNIW;
# then the VEX instructions of AVX-VNNI-INT8/INT16, AVX-IFMA, AVX-NE-CONVERT,
# SHA512, SM3, SM4, CMPccXADD, FMA4 and the kshift mask instructions.
# One table line each (asm/x64.ax, "The instruction table", says how one
# reads), in Intel SDM vol. 2's terms. The encodings (map, prefix, W, opcode,
# ModRM digit), the operand shapes, the EVEX tuples (disp8 scales), broadcast
# sizes, masking, and rounding or {sae} were taken in development from a
# reference encoder's output for every encoding of each map, never written by
# hand, and are checked by the table tests (cmd/axiom/internal/tests/c/run/
# asm_table_x64_more.c). LEGACY_FORMS is for legacy-map instructions the
# hand-written encoder lacks (none yet).

EVEX_FORMS = """
kmovb RM r32,k E.66.0f.w0 93 /r lx
kmovd RM r32,k E.f2.0f.w0 93 /r lx
kmovq RM r64,k E.f2.0f.w1 93 /r lx
kmovw RM r32,k E.np.0f.w0 93 /r lx
v4fmaddps RVM z,z,m E.f2.0f38.w0 9a /r lz n16 z
v4fmaddss RVM x,x,m E.f2.0f38.w0 9b /r lig n16 z
v4fnmaddps RVM z,z,m E.f2.0f38.w0 aa /r lz n16 z
v4fnmaddss RVM x,x,m E.f2.0f38.w0 ab /r lig n16 z
vaddph RVM x,x,x/m E.np.m5.w0 58 /r lx fv b2 z er
vaddph RVM y,y,y/m E.np.m5.w0 58 /r ly fv b2 z er
vaddph RVM z,z,z/m E.np.m5.w0 58 /r lz fv b2 z er
vaddsh RVM x,x,x/m E.f3.m5.w0 58 /r lig n2 z er
valignd RVMI x,x,x/m,i E.66.0f3a.w0 03 /r lx fv b4 z
valignd RVMI y,y,y/m,i E.66.0f3a.w0 03 /r ly fv b4 z
valignd RVMI z,z,z/m,i E.66.0f3a.w0 03 /r lz fv b4 z
valignq RVMI x,x,x/m,i E.66.0f3a.w1 03 /r lx fv b8 z
valignq RVMI y,y,y/m,i E.66.0f3a.w1 03 /r ly fv b8 z
valignq RVMI z,z,z/m,i E.66.0f3a.w1 03 /r lz fv b8 z
vblendmpd RVM x,x,x/m E.66.0f38.w1 65 /r lx fv b8 z
vblendmpd RVM y,y,y/m E.66.0f38.w1 65 /r ly fv b8 z
vblendmpd RVM z,z,z/m E.66.0f38.w1 65 /r lz fv b8 z
vblendmps RVM x,x,x/m E.66.0f38.w0 65 /r lx fv b4 z
vblendmps RVM y,y,y/m E.66.0f38.w0 65 /r ly fv b4 z
vblendmps RVM z,z,z/m E.66.0f38.w0 65 /r lz fv b4 z
vbroadcastf32x2 RM y,x/m E.66.0f38.w0 19 /r ly n8 z
vbroadcastf32x2 RM z,x/m E.66.0f38.w0 19 /r lz n8 z
vbroadcastf32x4 RM y,m E.66.0f38.w0 1a /r ly n16 z
vbroadcastf32x4 RM z,m E.66.0f38.w0 1a /r lz n16 z
vbroadcastf32x8 RM z,m E.66.0f38.w0 1b /r lz n32 z
vbroadcastf64x2 RM y,m E.66.0f38.w1 1a /r ly n16 z
vbroadcastf64x2 RM z,m E.66.0f38.w1 1a /r lz n16 z
vbroadcastf64x4 RM z,m E.66.0f38.w1 1b /r lz n32 z
vbroadcasti32x2 RM x,x/m E.66.0f38.w0 59 /r lx n8 z
vbroadcasti32x2 RM y,x/m E.66.0f38.w0 59 /r ly n8 z
vbroadcasti32x2 RM z,x/m E.66.0f38.w0 59 /r lz n8 z
vbroadcasti32x4 RM y,m E.66.0f38.w0 5a /r ly n16 z
vbroadcasti32x4 RM z,m E.66.0f38.w0 5a /r lz n16 z
vbroadcasti32x8 RM z,m E.66.0f38.w0 5b /r lz n32 z
vbroadcasti64x2 RM y,m E.66.0f38.w1 5a /r ly n16 z
vbroadcasti64x2 RM z,m E.66.0f38.w1 5a /r lz n16 z
vbroadcasti64x4 RM z,m E.66.0f38.w1 5b /r lz n32 z
vcmpph RVMI k,x,x/m,i E.np.0f3a.w0 c2 /r lx fv b2 k sae
vcmpph RVMI k,y,y/m,i E.np.0f3a.w0 c2 /r ly fv b2 k sae
vcmpph RVMI k,z,z/m,i E.np.0f3a.w0 c2 /r lz fv b2 k sae
vcmpsh RVMI k,x,x/m,i E.f3.0f3a.w0 c2 /r lig n2 k sae
vcomish RM x,x/m E.np.m5.w0 2f /r lig n2 sae
vcvtdq2pd RM z,y/m E.f3.0f.w0 e6 /r lz hv b4 z
vcvtdq2ph RM x,x/m E.np.m5.w0 5b /r lx fv b4 z er
vcvtdq2ph RM x,y/m E.np.m5.w0 5b /r ly fv b4 z er
vcvtdq2ph RM y,z/m E.np.m5.w0 5b /r lz fv b4 z er
vcvtdq2phx RM x,m E.np.m5.w0 5b /r lx n16 z
vcvtdq2phy RM x,m E.np.m5.w0 5b /r ly n32 z
vcvtne2ps2bf16 RVM x,x,x/m E.f2.0f38.w0 72 /r lx fv b4 z
vcvtne2ps2bf16 RVM y,y,y/m E.f2.0f38.w0 72 /r ly fv b4 z
vcvtne2ps2bf16 RVM z,z,z/m E.f2.0f38.w0 72 /r lz fv b4 z
vcvtneps2bf16 RM x,x/m E.f3.0f38.w0 72 /r lx fv b4 z
vcvtneps2bf16 RM x,y/m E.f3.0f38.w0 72 /r ly fv b4 z
vcvtneps2bf16 RM y,z/m E.f3.0f38.w0 72 /r lz fv b4 z
vcvtneps2bf16x RM x,m E.f3.0f38.w0 72 /r lx n16 z
vcvtneps2bf16y RM x,m E.f3.0f38.w0 72 /r ly n32 z
vcvtpd2dq RM y,z/m E.f2.0f.w1 e6 /r lz fv b8 z er
vcvtpd2ph RM x,x/m E.66.m5.w1 5a /r lx fv b8 z er
vcvtpd2ph RM x,y/m E.66.m5.w1 5a /r ly fv b8 z er
vcvtpd2ph RM x,z/m E.66.m5.w1 5a /r lz fv b8 z er
vcvtpd2phx RM x,m E.66.m5.w1 5a /r lx n16 z
vcvtpd2phy RM x,m E.66.m5.w1 5a /r ly n32 z
vcvtpd2phz RM x,m E.66.m5.w1 5a /r lz n64 z
vcvtpd2qq RM x,x/m E.66.0f.w1 7b /r lx fv b8 z er
vcvtpd2qq RM y,y/m E.66.0f.w1 7b /r ly fv b8 z er
vcvtpd2qq RM z,z/m E.66.0f.w1 7b /r lz fv b8 z er
vcvtpd2udq RM x,x/m E.np.0f.w1 79 /r lx fv b8 z er
vcvtpd2udq RM x,y/m E.np.0f.w1 79 /r ly fv b8 z er
vcvtpd2udq RM y,z/m E.np.0f.w1 79 /r lz fv b8 z er
vcvtpd2udqx RM x,m E.np.0f.w1 79 /r lx n16 z
vcvtpd2udqy RM x,m E.np.0f.w1 79 /r ly n32 z
vcvtpd2uqq RM x,x/m E.66.0f.w1 79 /r lx fv b8 z er
vcvtpd2uqq RM y,y/m E.66.0f.w1 79 /r ly fv b8 z er
vcvtpd2uqq RM z,z/m E.66.0f.w1 79 /r lz fv b8 z er
vcvtph2dq RM x,x/m E.66.m5.w0 5b /r lx hv b2 z er
vcvtph2dq RM y,x/m E.66.m5.w0 5b /r ly hv b2 z er
vcvtph2dq RM z,y/m E.66.m5.w0 5b /r lz hv b2 z er
vcvtph2pd RM x,x/m E.np.m5.w0 5a /r lx qv b2 z sae
vcvtph2pd RM y,x/m E.np.m5.w0 5a /r ly qv b2 z sae
vcvtph2pd RM z,x/m E.np.m5.w0 5a /r lz qv b2 z sae
vcvtph2psx RM x,x/m E.66.m6.w0 13 /r lx hv b2 z sae
vcvtph2psx RM y,x/m E.66.m6.w0 13 /r ly hv b2 z sae
vcvtph2psx RM z,y/m E.66.m6.w0 13 /r lz hv b2 z sae
vcvtph2qq RM x,x/m E.66.m5.w0 7b /r lx qv b2 z er
vcvtph2qq RM y,x/m E.66.m5.w0 7b /r ly qv b2 z er
vcvtph2qq RM z,x/m E.66.m5.w0 7b /r lz qv b2 z er
vcvtph2udq RM x,x/m E.np.m5.w0 79 /r lx hv b2 z er
vcvtph2udq RM y,x/m E.np.m5.w0 79 /r ly hv b2 z er
vcvtph2udq RM z,y/m E.np.m5.w0 79 /r lz hv b2 z er
vcvtph2uqq RM x,x/m E.66.m5.w0 79 /r lx qv b2 z er
vcvtph2uqq RM y,x/m E.66.m5.w0 79 /r ly qv b2 z er
vcvtph2uqq RM z,x/m E.66.m5.w0 79 /r lz qv b2 z er
vcvtph2uw RM x,x/m E.np.m5.w0 7d /r lx fv b2 z er
vcvtph2uw RM y,y/m E.np.m5.w0 7d /r ly fv b2 z er
vcvtph2uw RM z,z/m E.np.m5.w0 7d /r lz fv b2 z er
vcvtph2w RM x,x/m E.66.m5.w0 7d /r lx fv b2 z er
vcvtph2w RM y,y/m E.66.m5.w0 7d /r ly fv b2 z er
vcvtph2w RM z,z/m E.66.m5.w0 7d /r lz fv b2 z er
vcvtps2phx RM x,x/m E.66.m5.w0 1d /r lx fv b4 z er
vcvtps2phx RM x,y/m E.66.m5.w0 1d /r ly fv b4 z er
vcvtps2phx RM y,z/m E.66.m5.w0 1d /r lz fv b4 z er
vcvtps2phxx RM x,m E.66.m5.w0 1d /r lx n16 z
vcvtps2phxy RM x,m E.66.m5.w0 1d /r ly n32 z
vcvtps2qq RM x,x/m E.66.0f.w0 7b /r lx hv b4 z er
vcvtps2qq RM y,x/m E.66.0f.w0 7b /r ly hv b4 z er
vcvtps2qq RM z,y/m E.66.0f.w0 7b /r lz hv b4 z er
vcvtps2udq RM x,x/m E.np.0f.w0 79 /r lx fv b4 z er
vcvtps2udq RM y,y/m E.np.0f.w0 79 /r ly fv b4 z er
vcvtps2udq RM z,z/m E.np.0f.w0 79 /r lz fv b4 z er
vcvtps2uqq RM x,x/m E.66.0f.w0 79 /r lx hv b4 z er
vcvtps2uqq RM y,x/m E.66.0f.w0 79 /r ly hv b4 z er
vcvtps2uqq RM z,y/m E.66.0f.w0 79 /r lz hv b4 z er
vcvtqq2pd RM x,x/m E.f3.0f.w1 e6 /r lx fv b8 z er
vcvtqq2pd RM y,y/m E.f3.0f.w1 e6 /r ly fv b8 z er
vcvtqq2pd RM z,z/m E.f3.0f.w1 e6 /r lz fv b8 z er
vcvtqq2ph RM x,x/m E.np.m5.w1 5b /r lx fv b8 z er
vcvtqq2ph RM x,y/m E.np.m5.w1 5b /r ly fv b8 z er
vcvtqq2ph RM x,z/m E.np.m5.w1 5b /r lz fv b8 z er
vcvtqq2phx RM x,m E.np.m5.w1 5b /r lx n16 z
vcvtqq2phy RM x,m E.np.m5.w1 5b /r ly n32 z
vcvtqq2phz RM x,m E.np.m5.w1 5b /r lz n64 z
vcvtqq2ps RM x,x/m E.np.0f.w1 5b /r lx fv b8 z er
vcvtqq2ps RM x,y/m E.np.0f.w1 5b /r ly fv b8 z er
vcvtqq2ps RM y,z/m E.np.0f.w1 5b /r lz fv b8 z er
vcvtqq2psx RM x,m E.np.0f.w1 5b /r lx n16 z
vcvtqq2psy RM x,m E.np.0f.w1 5b /r ly n32 z
vcvtsd2sh RVM x,x,x/m E.f2.m5.w1 5a /r lig n8 z er
vcvtsd2si RM r32,x/m E.f2.0f.w0 2d /r lig n8 er
vcvtsd2si RM r64,x/m E.f2.0f.w1 2d /r lig n8 er
vcvtsd2usi RM r32,x/m E.f2.0f.w0 79 /r lig n8 er
vcvtsd2usi RM r64,x/m E.f2.0f.w1 79 /r lig n8 er
vcvtsh2sd RVM x,x,x/m E.f3.m5.w0 5a /r lig n2 z sae
vcvtsh2si RM r32,x/m E.f3.m5.w0 2d /r lig n2 er
vcvtsh2si RM r64,x/m E.f3.m5.w1 2d /r lig n2 er
vcvtsh2ss RVM x,x,x/m E.np.m6.w0 13 /r lig n2 z sae
vcvtsh2usi RM r32,x/m E.f3.m5.w0 79 /r lig n2 er
vcvtsh2usi RM r64,x/m E.f3.m5.w1 79 /r lig n2 er
vcvtsi2sd RVM x,x,r32 E.f2.0f.w0 2a /r lig
vcvtsi2sd RVM x,x,r64 E.f2.0f.w1 2a /r lig er
vcvtsi2sdl RVM x,x,m E.f2.0f.w0 2a /r lig n4
vcvtsi2sdq RVM x,x,m E.f2.0f.w1 2a /r lig n8
vcvtsi2sh RVM x,x,r32 E.f3.m5.w0 2a /r lig er
vcvtsi2sh RVM x,x,r64 E.f3.m5.w1 2a /r lig er
vcvtsi2shl RVM x,x,m E.f3.m5.w0 2a /r lig n4
vcvtsi2shq RVM x,x,m E.f3.m5.w1 2a /r lig n8
vcvtsi2ss RVM x,x,r32 E.f3.0f.w0 2a /r lig er
vcvtsi2ss RVM x,x,r64 E.f3.0f.w1 2a /r lig er
vcvtsi2ssl RVM x,x,m E.f3.0f.w0 2a /r lig n4
vcvtsi2ssq RVM x,x,m E.f3.0f.w1 2a /r lig n8
vcvtss2sh RVM x,x,x/m E.np.m5.w0 1d /r lig n4 z er
vcvtss2si RM r32,x/m E.f3.0f.w0 2d /r lig n4 er
vcvtss2si RM r64,x/m E.f3.0f.w1 2d /r lig n4 er
vcvtss2usi RM r32,x/m E.f3.0f.w0 79 /r lig n4 er
vcvtss2usi RM r64,x/m E.f3.0f.w1 79 /r lig n4 er
vcvttpd2dq RM y,z/m E.66.0f.w1 e6 /r lz fv b8 z sae
vcvttpd2qq RM x,x/m E.66.0f.w1 7a /r lx fv b8 z sae
vcvttpd2qq RM y,y/m E.66.0f.w1 7a /r ly fv b8 z sae
vcvttpd2qq RM z,z/m E.66.0f.w1 7a /r lz fv b8 z sae
vcvttpd2udq RM x,x/m E.np.0f.w1 78 /r lx fv b8 z sae
vcvttpd2udq RM x,y/m E.np.0f.w1 78 /r ly fv b8 z sae
vcvttpd2udq RM y,z/m E.np.0f.w1 78 /r lz fv b8 z sae
vcvttpd2udqx RM x,m E.np.0f.w1 78 /r lx n16 z
vcvttpd2udqy RM x,m E.np.0f.w1 78 /r ly n32 z
vcvttpd2uqq RM x,x/m E.66.0f.w1 78 /r lx fv b8 z sae
vcvttpd2uqq RM y,y/m E.66.0f.w1 78 /r ly fv b8 z sae
vcvttpd2uqq RM z,z/m E.66.0f.w1 78 /r lz fv b8 z sae
vcvttph2dq RM x,x/m E.f3.m5.w0 5b /r lx hv b2 z sae
vcvttph2dq RM y,x/m E.f3.m5.w0 5b /r ly hv b2 z sae
vcvttph2dq RM z,y/m E.f3.m5.w0 5b /r lz hv b2 z sae
vcvttph2qq RM x,x/m E.66.m5.w0 7a /r lx qv b2 z sae
vcvttph2qq RM y,x/m E.66.m5.w0 7a /r ly qv b2 z sae
vcvttph2qq RM z,x/m E.66.m5.w0 7a /r lz qv b2 z sae
vcvttph2udq RM x,x/m E.np.m5.w0 78 /r lx hv b2 z sae
vcvttph2udq RM y,x/m E.np.m5.w0 78 /r ly hv b2 z sae
vcvttph2udq RM z,y/m E.np.m5.w0 78 /r lz hv b2 z sae
vcvttph2uqq RM x,x/m E.66.m5.w0 78 /r lx qv b2 z sae
vcvttph2uqq RM y,x/m E.66.m5.w0 78 /r ly qv b2 z sae
vcvttph2uqq RM z,x/m E.66.m5.w0 78 /r lz qv b2 z sae
vcvttph2uw RM x,x/m E.np.m5.w0 7c /r lx fv b2 z sae
vcvttph2uw RM y,y/m E.np.m5.w0 7c /r ly fv b2 z sae
vcvttph2uw RM z,z/m E.np.m5.w0 7c /r lz fv b2 z sae
vcvttph2w RM x,x/m E.66.m5.w0 7c /r lx fv b2 z sae
vcvttph2w RM y,y/m E.66.m5.w0 7c /r ly fv b2 z sae
vcvttph2w RM z,z/m E.66.m5.w0 7c /r lz fv b2 z sae
vcvttps2qq RM x,x/m E.66.0f.w0 7a /r lx hv b4 z sae
vcvttps2qq RM y,x/m E.66.0f.w0 7a /r ly hv b4 z sae
vcvttps2qq RM z,y/m E.66.0f.w0 7a /r lz hv b4 z sae
vcvttps2udq RM x,x/m E.np.0f.w0 78 /r lx fv b4 z sae
vcvttps2udq RM y,y/m E.np.0f.w0 78 /r ly fv b4 z sae
vcvttps2udq RM z,z/m E.np.0f.w0 78 /r lz fv b4 z sae
vcvttps2uqq RM x,x/m E.66.0f.w0 78 /r lx hv b4 z sae
vcvttps2uqq RM y,x/m E.66.0f.w0 78 /r ly hv b4 z sae
vcvttps2uqq RM z,y/m E.66.0f.w0 78 /r lz hv b4 z sae
vcvttsd2si RM r32,x/m E.f2.0f.w0 2c /r lig n8 sae
vcvttsd2si RM r64,x/m E.f2.0f.w1 2c /r lig n8 sae
vcvttsd2usi RM r32,x/m E.f2.0f.w0 78 /r lig n8 sae
vcvttsd2usi RM r64,x/m E.f2.0f.w1 78 /r lig n8 sae
vcvttsh2si RM r32,x/m E.f3.m5.w0 2c /r lig n2 sae
vcvttsh2si RM r64,x/m E.f3.m5.w1 2c /r lig n2 sae
vcvttsh2usi RM r32,x/m E.f3.m5.w0 78 /r lig n2 sae
vcvttsh2usi RM r64,x/m E.f3.m5.w1 78 /r lig n2 sae
vcvttss2si RM r32,x/m E.f3.0f.w0 2c /r lig n4 sae
vcvttss2si RM r64,x/m E.f3.0f.w1 2c /r lig n4 sae
vcvttss2usi RM r32,x/m E.f3.0f.w0 78 /r lig n4 sae
vcvttss2usi RM r64,x/m E.f3.0f.w1 78 /r lig n4 sae
vcvtudq2pd RM x,x/m E.f3.0f.w0 7a /r lx hv b4 z
vcvtudq2pd RM y,x/m E.f3.0f.w0 7a /r ly hv b4 z
vcvtudq2pd RM z,y/m E.f3.0f.w0 7a /r lz hv b4 z
vcvtudq2ph RM x,x/m E.f2.m5.w0 7a /r lx fv b4 z er
vcvtudq2ph RM x,y/m E.f2.m5.w0 7a /r ly fv b4 z er
vcvtudq2ph RM y,z/m E.f2.m5.w0 7a /r lz fv b4 z er
vcvtudq2phx RM x,m E.f2.m5.w0 7a /r lx n16 z
vcvtudq2phy RM x,m E.f2.m5.w0 7a /r ly n32 z
vcvtudq2ps RM x,x/m E.f2.0f.w0 7a /r lx fv b4 z er
vcvtudq2ps RM y,y/m E.f2.0f.w0 7a /r ly fv b4 z er
vcvtudq2ps RM z,z/m E.f2.0f.w0 7a /r lz fv b4 z er
vcvtuqq2pd RM x,x/m E.f3.0f.w1 7a /r lx fv b8 z er
vcvtuqq2pd RM y,y/m E.f3.0f.w1 7a /r ly fv b8 z er
vcvtuqq2pd RM z,z/m E.f3.0f.w1 7a /r lz fv b8 z er
vcvtuqq2ph RM x,x/m E.f2.m5.w1 7a /r lx fv b8 z er
vcvtuqq2ph RM x,y/m E.f2.m5.w1 7a /r ly fv b8 z er
vcvtuqq2ph RM x,z/m E.f2.m5.w1 7a /r lz fv b8 z er
vcvtuqq2phx RM x,m E.f2.m5.w1 7a /r lx n16 z
vcvtuqq2phy RM x,m E.f2.m5.w1 7a /r ly n32 z
vcvtuqq2phz RM x,m E.f2.m5.w1 7a /r lz n64 z
vcvtuqq2ps RM x,x/m E.f2.0f.w1 7a /r lx fv b8 z er
vcvtuqq2ps RM x,y/m E.f2.0f.w1 7a /r ly fv b8 z er
vcvtuqq2ps RM y,z/m E.f2.0f.w1 7a /r lz fv b8 z er
vcvtuqq2psx RM x,m E.f2.0f.w1 7a /r lx n16 z
vcvtuqq2psy RM x,m E.f2.0f.w1 7a /r ly n32 z
vcvtusi2sd RVM x,x,r32 E.f2.0f.w0 7b /r lig
vcvtusi2sd RVM x,x,r64 E.f2.0f.w1 7b /r lig er
vcvtusi2sdl RVM x,x,m E.f2.0f.w0 7b /r lig n4
vcvtusi2sdq RVM x,x,m E.f2.0f.w1 7b /r lig n8
vcvtusi2sh RVM x,x,r32 E.f3.m5.w0 7b /r lig er
vcvtusi2sh RVM x,x,r64 E.f3.m5.w1 7b /r lig er
vcvtusi2shl RVM x,x,m E.f3.m5.w0 7b /r lig n4
vcvtusi2shq RVM x,x,m E.f3.m5.w1 7b /r lig n8
vcvtusi2ss RVM x,x,r32 E.f3.0f.w0 7b /r lig er
vcvtusi2ss RVM x,x,r64 E.f3.0f.w1 7b /r lig er
vcvtusi2ssl RVM x,x,m E.f3.0f.w0 7b /r lig n4
vcvtusi2ssq RVM x,x,m E.f3.0f.w1 7b /r lig n8
vcvtuw2ph RM x,x/m E.f2.m5.w0 7d /r lx fv b2 z er
vcvtuw2ph RM y,y/m E.f2.m5.w0 7d /r ly fv b2 z er
vcvtuw2ph RM z,z/m E.f2.m5.w0 7d /r lz fv b2 z er
vcvtw2ph RM x,x/m E.f3.m5.w0 7d /r lx fv b2 z er
vcvtw2ph RM y,y/m E.f3.m5.w0 7d /r ly fv b2 z er
vcvtw2ph RM z,z/m E.f3.m5.w0 7d /r lz fv b2 z er
vdbpsadbw RVMI x,x,x/m,i E.66.0f3a.w0 42 /r lx n16 z
vdbpsadbw RVMI x,x,x/m,i E.f2.0f3a.w0 42 /r lx n16 z
vdbpsadbw RVMI x,x,x/m,i E.f3.0f3a.w0 42 /r lx n16 z
vdbpsadbw RVMI x,x,x/m,i E.np.0f3a.w0 42 /r lx n16 z
vdbpsadbw RVMI y,y,y/m,i E.66.0f3a.w0 42 /r ly n32 z
vdbpsadbw RVMI y,y,y/m,i E.f2.0f3a.w0 42 /r ly n32 z
vdbpsadbw RVMI y,y,y/m,i E.f3.0f3a.w0 42 /r ly n32 z
vdbpsadbw RVMI y,y,y/m,i E.np.0f3a.w0 42 /r ly n32 z
vdbpsadbw RVMI z,z,z/m,i E.66.0f3a.w0 42 /r lz n64 z
vdbpsadbw RVMI z,z,z/m,i E.f2.0f3a.w0 42 /r lz n64 z
vdbpsadbw RVMI z,z,z/m,i E.f3.0f3a.w0 42 /r lz n64 z
vdbpsadbw RVMI z,z,z/m,i E.np.0f3a.w0 42 /r lz n64 z
vdivph RVM x,x,x/m E.np.m5.w0 5e /r lx fv b2 z er
vdivph RVM y,y,y/m E.np.m5.w0 5e /r ly fv b2 z er
vdivph RVM z,z,z/m E.np.m5.w0 5e /r lz fv b2 z er
vdivsh RVM x,x,x/m E.f3.m5.w0 5e /r lig n2 z er
vdpbf16ps RVM x,x,x/m E.f3.0f38.w0 52 /r lx fv b4 z
vdpbf16ps RVM y,y,y/m E.f3.0f38.w0 52 /r ly fv b4 z
vdpbf16ps RVM z,z,z/m E.f3.0f38.w0 52 /r lz fv b4 z
vexp2pd RM z,z/m E.66.0f38.w1 c8 /r lz fv b8 z sae
vexp2ps RM z,z/m E.66.0f38.w0 c8 /r lz fv b4 z sae
vfcmaddcph RVM x,x,x/m E.f2.m6.w0 56 /r lx fv b4 z er
vfcmaddcph RVM y,y,y/m E.f2.m6.w0 56 /r ly fv b4 z er
vfcmaddcph RVM z,z,z/m E.f2.m6.w0 56 /r lz fv b4 z er
vfcmaddcsh RVM x,x,x/m E.f2.m6.w0 57 /r lig n4 z er
vfcmulcph RVM x,x,x/m E.f2.m6.w0 d6 /r lx fv b4 z er
vfcmulcph RVM y,y,y/m E.f2.m6.w0 d6 /r ly fv b4 z er
vfcmulcph RVM z,z,z/m E.f2.m6.w0 d6 /r lz fv b4 z er
vfcmulcsh RVM x,x,x/m E.f2.m6.w0 d7 /r lig n4 z er
vfixupimmpd RVMI x,x,x/m,i E.66.0f3a.w1 54 /r lx fv b8 z sae
vfixupimmpd RVMI y,y,y/m,i E.66.0f3a.w1 54 /r ly fv b8 z sae
vfixupimmpd RVMI z,z,z/m,i E.66.0f3a.w1 54 /r lz fv b8 z sae
vfixupimmps RVMI x,x,x/m,i E.66.0f3a.w0 54 /r lx fv b4 z sae
vfixupimmps RVMI y,y,y/m,i E.66.0f3a.w0 54 /r ly fv b4 z sae
vfixupimmps RVMI z,z,z/m,i E.66.0f3a.w0 54 /r lz fv b4 z sae
vfixupimmsd RVMI x,x,x/m,i E.66.0f3a.w1 55 /r lig n8 z sae
vfixupimmss RVMI x,x,x/m,i E.66.0f3a.w0 55 /r lig n4 z sae
vfmadd132ph RVM x,x,x/m E.66.m6.w0 98 /r lx fv b2 z er
vfmadd132ph RVM y,y,y/m E.66.m6.w0 98 /r ly fv b2 z er
vfmadd132ph RVM z,z,z/m E.66.m6.w0 98 /r lz fv b2 z er
vfmadd132sh RVM x,x,x/m E.66.m6.w0 99 /r lig n2 z er
vfmadd213ph RVM x,x,x/m E.66.m6.w0 a8 /r lx fv b2 z er
vfmadd213ph RVM y,y,y/m E.66.m6.w0 a8 /r ly fv b2 z er
vfmadd213ph RVM z,z,z/m E.66.m6.w0 a8 /r lz fv b2 z er
vfmadd213sh RVM x,x,x/m E.66.m6.w0 a9 /r lig n2 z er
vfmadd231ph RVM x,x,x/m E.66.m6.w0 b8 /r lx fv b2 z er
vfmadd231ph RVM y,y,y/m E.66.m6.w0 b8 /r ly fv b2 z er
vfmadd231ph RVM z,z,z/m E.66.m6.w0 b8 /r lz fv b2 z er
vfmadd231sh RVM x,x,x/m E.66.m6.w0 b9 /r lig n2 z er
vfmaddcph RVM x,x,x/m E.f3.m6.w0 56 /r lx fv b4 z er
vfmaddcph RVM y,y,y/m E.f3.m6.w0 56 /r ly fv b4 z er
vfmaddcph RVM z,z,z/m E.f3.m6.w0 56 /r lz fv b4 z er
vfmaddcsh RVM x,x,x/m E.f3.m6.w0 57 /r lig n4 z er
vfmaddsub132ph RVM x,x,x/m E.66.m6.w0 96 /r lx fv b2 z er
vfmaddsub132ph RVM y,y,y/m E.66.m6.w0 96 /r ly fv b2 z er
vfmaddsub132ph RVM z,z,z/m E.66.m6.w0 96 /r lz fv b2 z er
vfmaddsub213ph RVM x,x,x/m E.66.m6.w0 a6 /r lx fv b2 z er
vfmaddsub213ph RVM y,y,y/m E.66.m6.w0 a6 /r ly fv b2 z er
vfmaddsub213ph RVM z,z,z/m E.66.m6.w0 a6 /r lz fv b2 z er
vfmaddsub231ph RVM x,x,x/m E.66.m6.w0 b6 /r lx fv b2 z er
vfmaddsub231ph RVM y,y,y/m E.66.m6.w0 b6 /r ly fv b2 z er
vfmaddsub231ph RVM z,z,z/m E.66.m6.w0 b6 /r lz fv b2 z er
vfmsub132ph RVM x,x,x/m E.66.m6.w0 9a /r lx fv b2 z er
vfmsub132ph RVM y,y,y/m E.66.m6.w0 9a /r ly fv b2 z er
vfmsub132ph RVM z,z,z/m E.66.m6.w0 9a /r lz fv b2 z er
vfmsub132sh RVM x,x,x/m E.66.m6.w0 9b /r lig n2 z er
vfmsub213ph RVM x,x,x/m E.66.m6.w0 aa /r lx fv b2 z er
vfmsub213ph RVM y,y,y/m E.66.m6.w0 aa /r ly fv b2 z er
vfmsub213ph RVM z,z,z/m E.66.m6.w0 aa /r lz fv b2 z er
vfmsub213sh RVM x,x,x/m E.66.m6.w0 ab /r lig n2 z er
vfmsub231ph RVM x,x,x/m E.66.m6.w0 ba /r lx fv b2 z er
vfmsub231ph RVM y,y,y/m E.66.m6.w0 ba /r ly fv b2 z er
vfmsub231ph RVM z,z,z/m E.66.m6.w0 ba /r lz fv b2 z er
vfmsub231sh RVM x,x,x/m E.66.m6.w0 bb /r lig n2 z er
vfmsubadd132ph RVM x,x,x/m E.66.m6.w0 97 /r lx fv b2 z er
vfmsubadd132ph RVM y,y,y/m E.66.m6.w0 97 /r ly fv b2 z er
vfmsubadd132ph RVM z,z,z/m E.66.m6.w0 97 /r lz fv b2 z er
vfmsubadd213ph RVM x,x,x/m E.66.m6.w0 a7 /r lx fv b2 z er
vfmsubadd213ph RVM y,y,y/m E.66.m6.w0 a7 /r ly fv b2 z er
vfmsubadd213ph RVM z,z,z/m E.66.m6.w0 a7 /r lz fv b2 z er
vfmsubadd231ph RVM x,x,x/m E.66.m6.w0 b7 /r lx fv b2 z er
vfmsubadd231ph RVM y,y,y/m E.66.m6.w0 b7 /r ly fv b2 z er
vfmsubadd231ph RVM z,z,z/m E.66.m6.w0 b7 /r lz fv b2 z er
vfmulcph RVM x,x,x/m E.f3.m6.w0 d6 /r lx fv b4 z er
vfmulcph RVM y,y,y/m E.f3.m6.w0 d6 /r ly fv b4 z er
vfmulcph RVM z,z,z/m E.f3.m6.w0 d6 /r lz fv b4 z er
vfmulcsh RVM x,x,x/m E.f3.m6.w0 d7 /r lig n4 z er
vfnmadd132ph RVM x,x,x/m E.66.m6.w0 9c /r lx fv b2 z er
vfnmadd132ph RVM y,y,y/m E.66.m6.w0 9c /r ly fv b2 z er
vfnmadd132ph RVM z,z,z/m E.66.m6.w0 9c /r lz fv b2 z er
vfnmadd132sh RVM x,x,x/m E.66.m6.w0 9d /r lig n2 z er
vfnmadd213ph RVM x,x,x/m E.66.m6.w0 ac /r lx fv b2 z er
vfnmadd213ph RVM y,y,y/m E.66.m6.w0 ac /r ly fv b2 z er
vfnmadd213ph RVM z,z,z/m E.66.m6.w0 ac /r lz fv b2 z er
vfnmadd213sh RVM x,x,x/m E.66.m6.w0 ad /r lig n2 z er
vfnmadd231ph RVM x,x,x/m E.66.m6.w0 bc /r lx fv b2 z er
vfnmadd231ph RVM y,y,y/m E.66.m6.w0 bc /r ly fv b2 z er
vfnmadd231ph RVM z,z,z/m E.66.m6.w0 bc /r lz fv b2 z er
vfnmadd231sh RVM x,x,x/m E.66.m6.w0 bd /r lig n2 z er
vfnmsub132ph RVM x,x,x/m E.66.m6.w0 9e /r lx fv b2 z er
vfnmsub132ph RVM y,y,y/m E.66.m6.w0 9e /r ly fv b2 z er
vfnmsub132ph RVM z,z,z/m E.66.m6.w0 9e /r lz fv b2 z er
vfnmsub132sh RVM x,x,x/m E.66.m6.w0 9f /r lig n2 z er
vfnmsub213ph RVM x,x,x/m E.66.m6.w0 ae /r lx fv b2 z er
vfnmsub213ph RVM y,y,y/m E.66.m6.w0 ae /r ly fv b2 z er
vfnmsub213ph RVM z,z,z/m E.66.m6.w0 ae /r lz fv b2 z er
vfnmsub213sh RVM x,x,x/m E.66.m6.w0 af /r lig n2 z er
vfnmsub231ph RVM x,x,x/m E.66.m6.w0 be /r lx fv b2 z er
vfnmsub231ph RVM y,y,y/m E.66.m6.w0 be /r ly fv b2 z er
vfnmsub231ph RVM z,z,z/m E.66.m6.w0 be /r lz fv b2 z er
vfnmsub231sh RVM x,x,x/m E.66.m6.w0 bf /r lig n2 z er
vfpclasspd RMI k,x/m,i E.66.0f3a.w1 66 /r lx fv b8 k
vfpclasspd RMI k,y/m,i E.66.0f3a.w1 66 /r ly fv b8 k
vfpclasspd RMI k,z/m,i E.66.0f3a.w1 66 /r lz fv b8 k
vfpclasspdx RMI k,m,i E.66.0f3a.w1 66 /r lx n16 k
vfpclasspdy RMI k,m,i E.66.0f3a.w1 66 /r ly n32 k
vfpclasspdz RMI k,m,i E.66.0f3a.w1 66 /r lz n64 k
vfpclassph RMI k,x/m,i E.np.0f3a.w0 66 /r lx fv b2 k
vfpclassph RMI k,y/m,i E.np.0f3a.w0 66 /r ly fv b2 k
vfpclassph RMI k,z/m,i E.np.0f3a.w0 66 /r lz fv b2 k
vfpclassphx RMI k,m,i E.np.0f3a.w0 66 /r lx n16 k
vfpclassphy RMI k,m,i E.np.0f3a.w0 66 /r ly n32 k
vfpclassphz RMI k,m,i E.np.0f3a.w0 66 /r lz n64 k
vfpclassps RMI k,x/m,i E.66.0f3a.w0 66 /r lx fv b4 k
vfpclassps RMI k,y/m,i E.66.0f3a.w0 66 /r ly fv b4 k
vfpclassps RMI k,z/m,i E.66.0f3a.w0 66 /r lz fv b4 k
vfpclasspsx RMI k,m,i E.66.0f3a.w0 66 /r lx n16 k
vfpclasspsy RMI k,m,i E.66.0f3a.w0 66 /r ly n32 k
vfpclasspsz RMI k,m,i E.66.0f3a.w0 66 /r lz n64 k
vfpclasssd RMI k,x/m,i E.66.0f3a.w1 67 /r lig n8 k
vfpclasssh RMI k,x/m,i E.np.0f3a.w0 67 /r lig n2 k
vfpclassss RMI k,x/m,i E.66.0f3a.w0 67 /r lig n4 k
vgetexppd RM x,x/m E.66.0f38.w1 42 /r lx fv b8 z sae
vgetexppd RM y,y/m E.66.0f38.w1 42 /r ly fv b8 z sae
vgetexppd RM z,z/m E.66.0f38.w1 42 /r lz fv b8 z sae
vgetexpph RM x,x/m E.66.m6.w0 42 /r lx fv b2 z sae
vgetexpph RM y,y/m E.66.m6.w0 42 /r ly fv b2 z sae
vgetexpph RM z,z/m E.66.m6.w0 42 /r lz fv b2 z sae
vgetexpps RM x,x/m E.66.0f38.w0 42 /r lx fv b4 z sae
vgetexpps RM y,y/m E.66.0f38.w0 42 /r ly fv b4 z sae
vgetexpps RM z,z/m E.66.0f38.w0 42 /r lz fv b4 z sae
vgetexpsd RVM x,x,x/m E.66.0f38.w1 43 /r lig n8 z sae
vgetexpsh RVM x,x,x/m E.66.m6.w0 43 /r lig n2 z sae
vgetexpss RVM x,x,x/m E.66.0f38.w0 43 /r lig n4 z sae
vgetmantpd RMI x,x/m,i E.66.0f3a.w1 26 /r lx fv b8 z sae
vgetmantpd RMI y,y/m,i E.66.0f3a.w1 26 /r ly fv b8 z sae
vgetmantpd RMI z,z/m,i E.66.0f3a.w1 26 /r lz fv b8 z sae
vgetmantph RMI x,x/m,i E.np.0f3a.w0 26 /r lx fv b2 z sae
vgetmantph RMI y,y/m,i E.np.0f3a.w0 26 /r ly fv b2 z sae
vgetmantph RMI z,z/m,i E.np.0f3a.w0 26 /r lz fv b2 z sae
vgetmantps RMI x,x/m,i E.66.0f3a.w0 26 /r lx fv b4 z sae
vgetmantps RMI y,y/m,i E.66.0f3a.w0 26 /r ly fv b4 z sae
vgetmantps RMI z,z/m,i E.66.0f3a.w0 26 /r lz fv b4 z sae
vgetmantsd RVMI x,x,x/m,i E.66.0f3a.w1 27 /r lig n8 z sae
vgetmantsh RVMI x,x,x/m,i E.np.0f3a.w0 27 /r lig n2 z sae
vgetmantss RVMI x,x,x/m,i E.66.0f3a.w0 27 /r lig n4 z sae
vmaxph RVM x,x,x/m E.np.m5.w0 5f /r lx fv b2 z sae
vmaxph RVM y,y,y/m E.np.m5.w0 5f /r ly fv b2 z sae
vmaxph RVM z,z,z/m E.np.m5.w0 5f /r lz fv b2 z sae
vmaxsh RVM x,x,x/m E.f3.m5.w0 5f /r lig n2 z sae
vminph RVM x,x,x/m E.np.m5.w0 5d /r lx fv b2 z sae
vminph RVM y,y,y/m E.np.m5.w0 5d /r ly fv b2 z sae
vminph RVM z,z,z/m E.np.m5.w0 5d /r lz fv b2 z sae
vminsh RVM x,x,x/m E.f3.m5.w0 5d /r lig n2 z sae
vmovddup RM z,z/m E.f2.0f.w1 12 /r lz n64 z
vmovntdq MR m,z E.66.0f.w0 e7 /r lz n64
vmovntdqa RM z,m E.66.0f38.w0 2a /r lz n64
vmovntpd MR m,z E.66.0f.w1 2b /r lz n64
vmovntps MR m,z E.np.0f.w0 2b /r lz n64
vmovsd RM x,m E.f2.0f.w1 10 /r lig n8 z
vmovsd RVM x,x,x E.f2.0f.w1 10 /r lig z
vmovsd MR m,x E.f2.0f.w1 11 /r lig n8 k
vmovsh RM x,m E.f3.m5.w0 10 /r lig n2 z
vmovsh RVM x,x,x E.f3.m5.w0 10 /r lig z
vmovsh MR m,x E.f3.m5.w0 11 /r lig n2 k
vmovsh MVR x,x,x E.f3.m5.w0 11 /r lig z
vmovshdup RM z,z/m E.f3.0f.w0 16 /r lz n64 z
vmovsldup RM z,z/m E.f3.0f.w0 12 /r lz n64 z
vmovss RM x,m E.f3.0f.w0 10 /r lig n4 z
vmovss RVM x,x,x E.f3.0f.w0 10 /r lig z
vmovss MR m,x E.f3.0f.w0 11 /r lig n4 k
vmovw RM x,r32/m E.66.m5.w0 6e /r lig n2
vmovw MR r32/m,x E.66.m5.w0 7e /r lig n2
vmulph RVM x,x,x/m E.np.m5.w0 59 /r lx fv b2 z er
vmulph RVM y,y,y/m E.np.m5.w0 59 /r ly fv b2 z er
vmulph RVM z,z,z/m E.np.m5.w0 59 /r lz fv b2 z er
vmulsh RVM x,x,x/m E.f3.m5.w0 59 /r lig n2 z er
vp2intersectd RVM k,x,x/m E.f2.0f38.w0 68 /r lx fv b4
vp2intersectd RVM k,y,y/m E.f2.0f38.w0 68 /r ly fv b4
vp2intersectd RVM k,z,z/m E.f2.0f38.w0 68 /r lz fv b4
vp2intersectq RVM k,x,x/m E.f2.0f38.w1 68 /r lx fv b8
vp2intersectq RVM k,y,y/m E.f2.0f38.w1 68 /r ly fv b8
vp2intersectq RVM k,z,z/m E.f2.0f38.w1 68 /r lz fv b8
vp4dpwssd RVM z,z,m E.f2.0f38.w0 52 /r lz n16 z
vp4dpwssds RVM z,z,m E.f2.0f38.w0 53 /r lz n16 z
vpackssdw RVM z,z,m E.66.0f.w0 6b /r lz fv b4 z
vpblendmb RVM x,x,x/m E.66.0f38.w0 66 /r lx n16 z
vpblendmb RVM y,y,y/m E.66.0f38.w0 66 /r ly n32 z
vpblendmb RVM z,z,z/m E.66.0f38.w0 66 /r lz n64 z
vpblendmd RVM x,x,x/m E.66.0f38.w0 64 /r lx fv b4 z
vpblendmd RVM y,y,y/m E.66.0f38.w0 64 /r ly fv b4 z
vpblendmd RVM z,z,z/m E.66.0f38.w0 64 /r lz fv b4 z
vpblendmq RVM x,x,x/m E.66.0f38.w1 64 /r lx fv b8 z
vpblendmq RVM y,y,y/m E.66.0f38.w1 64 /r ly fv b8 z
vpblendmq RVM z,z,z/m E.66.0f38.w1 64 /r lz fv b8 z
vpblendmw RVM x,x,x/m E.66.0f38.w1 66 /r lx n16 z
vpblendmw RVM y,y,y/m E.66.0f38.w1 66 /r ly n32 z
vpblendmw RVM z,z,z/m E.66.0f38.w1 66 /r lz n64 z
vpcompressb MR x/m,x E.66.0f38.w0 63 /r lx n1 z k
vpcompressb MR y/m,y E.66.0f38.w0 63 /r ly n1 z k
vpcompressb MR z/m,z E.66.0f38.w0 63 /r lz n1 k z
vpcompressw MR x/m,x E.66.0f38.w1 63 /r lx n2 z k
vpcompressw MR y/m,y E.66.0f38.w1 63 /r ly n2 z k
vpcompressw MR z/m,z E.66.0f38.w1 63 /r lz n2 k z
vpermilpd RMI z,z/m,i E.66.0f3a.w1 05 /r lz fv b8 z
vpermilpd RVM z,z,z/m E.66.0f38.w1 0d /r lz fv b8 z
vpermilps RMI z,z/m,i E.66.0f3a.w0 04 /r lz fv b4 z
vpermilps RVM z,z,z/m E.66.0f38.w0 0c /r lz fv b4 z
vpexpandb RM x,x/m E.66.0f38.w0 62 /r lx n1 z
vpexpandb RM y,y/m E.66.0f38.w0 62 /r ly n1 z
vpexpandb RM z,z/m E.66.0f38.w0 62 /r lz n1 z
vpexpandw RM x,x/m E.66.0f38.w1 62 /r lx n2 z
vpexpandw RM y,y/m E.66.0f38.w1 62 /r ly n2 z
vpexpandw RM z,z/m E.66.0f38.w1 62 /r lz n2 z
vpmaxsq RVM x,x,x/m E.66.0f38.w1 3d /r lx fv b8 z
vpmaxsq RVM y,y,y/m E.66.0f38.w1 3d /r ly fv b8 z
vpmaxsq RVM z,z,z/m E.66.0f38.w1 3d /r lz fv b8 z
vpmaxuq RVM x,x,x/m E.66.0f38.w1 3f /r lx fv b8 z
vpmaxuq RVM y,y,y/m E.66.0f38.w1 3f /r ly fv b8 z
vpmaxuq RVM z,z,z/m E.66.0f38.w1 3f /r lz fv b8 z
vpminsq RVM x,x,x/m E.66.0f38.w1 39 /r lx fv b8 z
vpminsq RVM y,y,y/m E.66.0f38.w1 39 /r ly fv b8 z
vpminsq RVM z,z,z/m E.66.0f38.w1 39 /r lz fv b8 z
vpminuq RVM x,x,x/m E.66.0f38.w1 3b /r lx fv b8 z
vpminuq RVM y,y,y/m E.66.0f38.w1 3b /r ly fv b8 z
vpminuq RVM z,z,z/m E.66.0f38.w1 3b /r lz fv b8 z
vpmovb2m RM k,x E.f3.0f38.w0 29 /r lx
vpmovb2m RM k,y E.f3.0f38.w0 29 /r ly
vpmovb2m RM k,z E.f3.0f38.w0 29 /r lz
vpmovd2m RM k,x E.f3.0f38.w0 39 /r lx
vpmovd2m RM k,y E.f3.0f38.w0 39 /r ly
vpmovd2m RM k,z E.f3.0f38.w0 39 /r lz
vpmovm2b RM x,k E.f3.0f38.w0 28 /r lx
vpmovm2b RM y,k E.f3.0f38.w0 28 /r ly
vpmovm2b RM z,k E.f3.0f38.w0 28 /r lz
vpmovm2d RM x,k E.f3.0f38.w0 38 /r lx
vpmovm2d RM y,k E.f3.0f38.w0 38 /r ly
vpmovm2d RM z,k E.f3.0f38.w0 38 /r lz
vpmovm2q RM x,k E.f3.0f38.w1 38 /r lx
vpmovm2q RM y,k E.f3.0f38.w1 38 /r ly
vpmovm2q RM z,k E.f3.0f38.w1 38 /r lz
vpmovm2w RM x,k E.f3.0f38.w1 28 /r lx
vpmovm2w RM y,k E.f3.0f38.w1 28 /r ly
vpmovm2w RM z,k E.f3.0f38.w1 28 /r lz
vpmovq2m RM k,x E.f3.0f38.w1 39 /r lx
vpmovq2m RM k,y E.f3.0f38.w1 39 /r ly
vpmovq2m RM k,z E.f3.0f38.w1 39 /r lz
vpmovsxbd RM z,x/m E.66.0f38.w0 21 /r lz n16 z
vpmovsxbq RM z,x/m E.66.0f38.w0 22 /r lz n8 z
vpmovsxdq RM z,y/m E.66.0f38.w0 25 /r lz n32 z
vpmovsxwd RM z,y/m E.66.0f38.w0 23 /r lz n32 z
vpmovsxwq RM z,x/m E.66.0f38.w0 24 /r lz n16 z
vpmovw2m RM k,x E.f3.0f38.w1 29 /r lx
vpmovw2m RM k,y E.f3.0f38.w1 29 /r ly
vpmovw2m RM k,z E.f3.0f38.w1 29 /r lz
vpmovzxbd RM z,x/m E.66.0f38.w0 31 /r lz n16 z
vpmovzxbq RM z,x/m E.66.0f38.w0 32 /r lz n8 z
vpmovzxdq RM z,y/m E.66.0f38.w0 35 /r lz n32 z
vpmovzxwd RM z,y/m E.66.0f38.w0 33 /r lz n32 z
vpmovzxwq RM z,x/m E.66.0f38.w0 34 /r lz n16 z
vpmullq RVM x,x,x/m E.66.0f38.w1 40 /r lx fv b8 z
vpmullq RVM y,y,y/m E.66.0f38.w1 40 /r ly fv b8 z
vpmullq RVM z,z,z/m E.66.0f38.w1 40 /r lz fv b8 z
vprold VMI x,x/m,i E.66.0f.w0 72 /1 lx fv b4 z
vprold VMI y,y/m,i E.66.0f.w0 72 /1 ly fv b4 z
vprold VMI z,z/m,i E.66.0f.w0 72 /1 lz fv b4 z
vprolq VMI x,x/m,i E.66.0f.w1 72 /1 lx fv b8 z
vprolq VMI y,y/m,i E.66.0f.w1 72 /1 ly fv b8 z
vprolq VMI z,z/m,i E.66.0f.w1 72 /1 lz fv b8 z
vprolvd RVM x,x,x/m E.66.0f38.w0 15 /r lx fv b4 z
vprolvd RVM y,y,y/m E.66.0f38.w0 15 /r ly fv b4 z
vprolvd RVM z,z,z/m E.66.0f38.w0 15 /r lz fv b4 z
vprolvq RVM x,x,x/m E.66.0f38.w1 15 /r lx fv b8 z
vprolvq RVM y,y,y/m E.66.0f38.w1 15 /r ly fv b8 z
vprolvq RVM z,z,z/m E.66.0f38.w1 15 /r lz fv b8 z
vprord VMI x,x/m,i E.66.0f.w0 72 /0 lx fv b4 z
vprord VMI y,y/m,i E.66.0f.w0 72 /0 ly fv b4 z
vprord VMI z,z/m,i E.66.0f.w0 72 /0 lz fv b4 z
vprorq VMI x,x/m,i E.66.0f.w1 72 /0 lx fv b8 z
vprorq VMI y,y/m,i E.66.0f.w1 72 /0 ly fv b8 z
vprorq VMI z,z/m,i E.66.0f.w1 72 /0 lz fv b8 z
vprorvd RVM x,x,x/m E.66.0f38.w0 14 /r lx fv b4 z
vprorvd RVM y,y,y/m E.66.0f38.w0 14 /r ly fv b4 z
vprorvd RVM z,z,z/m E.66.0f38.w0 14 /r lz fv b4 z
vprorvq RVM x,x,x/m E.66.0f38.w1 14 /r lx fv b8 z
vprorvq RVM y,y,y/m E.66.0f38.w1 14 /r ly fv b8 z
vprorvq RVM z,z,z/m E.66.0f38.w1 14 /r lz fv b8 z
vpshldd RVMI x,x,x/m,i E.66.0f3a.w0 71 /r lx fv b4 z
vpshldd RVMI y,y,y/m,i E.66.0f3a.w0 71 /r ly fv b4 z
vpshldd RVMI z,z,z/m,i E.66.0f3a.w0 71 /r lz fv b4 z
vpshldq RVMI x,x,x/m,i E.66.0f3a.w1 71 /r lx fv b8 z
vpshldq RVMI y,y,y/m,i E.66.0f3a.w1 71 /r ly fv b8 z
vpshldq RVMI z,z,z/m,i E.66.0f3a.w1 71 /r lz fv b8 z
vpshldvd RVM x,x,x/m E.66.0f38.w0 71 /r lx fv b4 z
vpshldvd RVM y,y,y/m E.66.0f38.w0 71 /r ly fv b4 z
vpshldvd RVM z,z,z/m E.66.0f38.w0 71 /r lz fv b4 z
vpshldvq RVM x,x,x/m E.66.0f38.w1 71 /r lx fv b8 z
vpshldvq RVM y,y,y/m E.66.0f38.w1 71 /r ly fv b8 z
vpshldvq RVM z,z,z/m E.66.0f38.w1 71 /r lz fv b8 z
vpshldvw RVM x,x,x/m E.66.0f38.w1 70 /r lx n16 z
vpshldvw RVM y,y,y/m E.66.0f38.w1 70 /r ly n32 z
vpshldvw RVM z,z,z/m E.66.0f38.w1 70 /r lz n64 z
vpshldw RVMI x,x,x/m,i E.66.0f3a.w1 70 /r lx n16 z
vpshldw RVMI x,x,x/m,i E.f2.0f3a.w1 70 /r lx n16 z
vpshldw RVMI x,x,x/m,i E.f3.0f3a.w1 70 /r lx n16 z
vpshldw RVMI x,x,x/m,i E.np.0f3a.w1 70 /r lx n16 z
vpshldw RVMI y,y,y/m,i E.66.0f3a.w1 70 /r ly n32 z
vpshldw RVMI y,y,y/m,i E.f2.0f3a.w1 70 /r ly n32 z
vpshldw RVMI y,y,y/m,i E.f3.0f3a.w1 70 /r ly n32 z
vpshldw RVMI y,y,y/m,i E.np.0f3a.w1 70 /r ly n32 z
vpshldw RVMI z,z,z/m,i E.66.0f3a.w1 70 /r lz n64 z
vpshldw RVMI z,z,z/m,i E.f2.0f3a.w1 70 /r lz n64 z
vpshldw RVMI z,z,z/m,i E.f3.0f3a.w1 70 /r lz n64 z
vpshldw RVMI z,z,z/m,i E.np.0f3a.w1 70 /r lz n64 z
vpshrdd RVMI x,x,x/m,i E.66.0f3a.w0 73 /r lx fv b4 z
vpshrdd RVMI y,y,y/m,i E.66.0f3a.w0 73 /r ly fv b4 z
vpshrdd RVMI z,z,z/m,i E.66.0f3a.w0 73 /r lz fv b4 z
vpshrdq RVMI x,x,x/m,i E.66.0f3a.w1 73 /r lx fv b8 z
vpshrdq RVMI y,y,y/m,i E.66.0f3a.w1 73 /r ly fv b8 z
vpshrdq RVMI z,z,z/m,i E.66.0f3a.w1 73 /r lz fv b8 z
vpshrdvd RVM x,x,x/m E.66.0f38.w0 73 /r lx fv b4 z
vpshrdvd RVM y,y,y/m E.66.0f38.w0 73 /r ly fv b4 z
vpshrdvd RVM z,z,z/m E.66.0f38.w0 73 /r lz fv b4 z
vpshrdvq RVM x,x,x/m E.66.0f38.w1 73 /r lx fv b8 z
vpshrdvq RVM y,y,y/m E.66.0f38.w1 73 /r ly fv b8 z
vpshrdvq RVM z,z,z/m E.66.0f38.w1 73 /r lz fv b8 z
vpshrdvw RVM x,x,x/m E.66.0f38.w1 72 /r lx n16 z
vpshrdvw RVM y,y,y/m E.66.0f38.w1 72 /r ly n32 z
vpshrdvw RVM z,z,z/m E.66.0f38.w1 72 /r lz n64 z
vpshrdw RVMI x,x,x/m,i E.66.0f3a.w1 72 /r lx n16 z
vpshrdw RVMI x,x,x/m,i E.f2.0f3a.w1 72 /r lx n16 z
vpshrdw RVMI x,x,x/m,i E.f3.0f3a.w1 72 /r lx n16 z
vpshrdw RVMI x,x,x/m,i E.np.0f3a.w1 72 /r lx n16 z
vpshrdw RVMI y,y,y/m,i E.66.0f3a.w1 72 /r ly n32 z
vpshrdw RVMI y,y,y/m,i E.f2.0f3a.w1 72 /r ly n32 z
vpshrdw RVMI y,y,y/m,i E.f3.0f3a.w1 72 /r ly n32 z
vpshrdw RVMI y,y,y/m,i E.np.0f3a.w1 72 /r ly n32 z
vpshrdw RVMI z,z,z/m,i E.66.0f3a.w1 72 /r lz n64 z
vpshrdw RVMI z,z,z/m,i E.f2.0f3a.w1 72 /r lz n64 z
vpshrdw RVMI z,z,z/m,i E.f3.0f3a.w1 72 /r lz n64 z
vpshrdw RVMI z,z,z/m,i E.np.0f3a.w1 72 /r lz n64 z
vpshufhw RMI z,z/m,i E.f3.0f.w0 70 /r lz n64 z
vpshuflw RMI z,z/m,i E.f2.0f.w0 70 /r lz n64 z
vpslld RVM z,z,x/m E.66.0f.w0 f2 /r lz n16 z
vpsllq RVM z,z,x/m E.66.0f.w1 f3 /r lz n16 z
vpsllvw RVM x,x,x/m E.66.0f38.w1 12 /r lx n16 z
vpsllvw RVM y,y,y/m E.66.0f38.w1 12 /r ly n32 z
vpsllvw RVM z,z,z/m E.66.0f38.w1 12 /r lz n64 z
vpsllw RVM z,z,x/m E.66.0f.w0 f1 /r lz n16 z
vpsrad RVM z,z,x/m E.66.0f.w0 e2 /r lz n16 z
vpsraq RVM x,x,x/m E.66.0f.w1 e2 /r lx n16 z
vpsraq RVM y,y,x/m E.66.0f.w1 e2 /r ly n16 z
vpsraq RVM z,z,x/m E.66.0f.w1 e2 /r lz n16 z
vpsravw RVM x,x,x/m E.66.0f38.w1 11 /r lx n16 z
vpsravw RVM y,y,y/m E.66.0f38.w1 11 /r ly n32 z
vpsravw RVM z,z,z/m E.66.0f38.w1 11 /r lz n64 z
vpsraw RVM z,z,x/m E.66.0f.w0 e1 /r lz n16 z
vpsrld RVM z,z,x/m E.66.0f.w0 d2 /r lz n16 z
vpsrlq RVM z,z,x/m E.66.0f.w1 d3 /r lz n16 z
vpsrlvw RVM x,x,x/m E.66.0f38.w1 10 /r lx n16 z
vpsrlvw RVM y,y,y/m E.66.0f38.w1 10 /r ly n32 z
vpsrlvw RVM z,z,z/m E.66.0f38.w1 10 /r lz n64 z
vpsrlw RVM z,z,x/m E.66.0f.w0 d1 /r lz n16 z
vptestmb RVM k,x,x/m E.66.0f38.w0 26 /r lx n16 k
vptestmb RVM k,y,y/m E.66.0f38.w0 26 /r ly n32 k
vptestmb RVM k,z,z/m E.66.0f38.w0 26 /r lz n64 k
vptestmd RVM k,x,x/m E.66.0f38.w0 27 /r lx fv b4 k
vptestmd RVM k,y,y/m E.66.0f38.w0 27 /r ly fv b4 k
vptestmd RVM k,z,z/m E.66.0f38.w0 27 /r lz fv b4 k
vptestmq RVM k,x,x/m E.66.0f38.w1 27 /r lx fv b8 k
vptestmq RVM k,y,y/m E.66.0f38.w1 27 /r ly fv b8 k
vptestmq RVM k,z,z/m E.66.0f38.w1 27 /r lz fv b8 k
vptestmw RVM k,x,x/m E.66.0f38.w1 26 /r lx n16 k
vptestmw RVM k,y,y/m E.66.0f38.w1 26 /r ly n32 k
vptestmw RVM k,z,z/m E.66.0f38.w1 26 /r lz n64 k
vptestnmb RVM k,x,x/m E.f3.0f38.w0 26 /r lx n16 k
vptestnmb RVM k,y,y/m E.f3.0f38.w0 26 /r ly n32 k
vptestnmb RVM k,z,z/m E.f3.0f38.w0 26 /r lz n64 k
vptestnmd RVM k,x,x/m E.f3.0f38.w0 27 /r lx fv b4 k
vptestnmd RVM k,y,y/m E.f3.0f38.w0 27 /r ly fv b4 k
vptestnmd RVM k,z,z/m E.f3.0f38.w0 27 /r lz fv b4 k
vptestnmq RVM k,x,x/m E.f3.0f38.w1 27 /r lx fv b8 k
vptestnmq RVM k,y,y/m E.f3.0f38.w1 27 /r ly fv b8 k
vptestnmq RVM k,z,z/m E.f3.0f38.w1 27 /r lz fv b8 k
vptestnmw RVM k,x,x/m E.f3.0f38.w1 26 /r lx n16 k
vptestnmw RVM k,y,y/m E.f3.0f38.w1 26 /r ly n32 k
vptestnmw RVM k,z,z/m E.f3.0f38.w1 26 /r lz n64 k
vrangepd RVMI x,x,x/m,i E.66.0f3a.w1 50 /r lx fv b8 z sae
vrangepd RVMI y,y,y/m,i E.66.0f3a.w1 50 /r ly fv b8 z sae
vrangepd RVMI z,z,z/m,i E.66.0f3a.w1 50 /r lz fv b8 z sae
vrangeps RVMI x,x,x/m,i E.66.0f3a.w0 50 /r lx fv b4 z sae
vrangeps RVMI y,y,y/m,i E.66.0f3a.w0 50 /r ly fv b4 z sae
vrangeps RVMI z,z,z/m,i E.66.0f3a.w0 50 /r lz fv b4 z sae
vrangesd RVMI x,x,x/m,i E.66.0f3a.w1 51 /r lig n8 z sae
vrangess RVMI x,x,x/m,i E.66.0f3a.w0 51 /r lig n4 z sae
vrcp14pd RM x,x/m E.66.0f38.w1 4c /r lx fv b8 z
vrcp14pd RM y,y/m E.66.0f38.w1 4c /r ly fv b8 z
vrcp14pd RM z,z/m E.66.0f38.w1 4c /r lz fv b8 z
vrcp14ps RM x,x/m E.66.0f38.w0 4c /r lx fv b4 z
vrcp14ps RM y,y/m E.66.0f38.w0 4c /r ly fv b4 z
vrcp14ps RM z,z/m E.66.0f38.w0 4c /r lz fv b4 z
vrcp14sd RVM x,x,x/m E.66.0f38.w1 4d /r lig n8 z
vrcp14ss RVM x,x,x/m E.66.0f38.w0 4d /r lig n4 z
vrcp28pd RM z,z/m E.66.0f38.w1 ca /r lz fv b8 z sae
vrcp28ps RM z,z/m E.66.0f38.w0 ca /r lz fv b4 z sae
vrcp28sd RVM x,x,x/m E.66.0f38.w1 cb /r lig n8 z sae
vrcp28ss RVM x,x,x/m E.66.0f38.w0 cb /r lig n4 z sae
vrcpph RM x,x/m E.66.m6.w0 4c /r lx fv b2 z
vrcpph RM y,y/m E.66.m6.w0 4c /r ly fv b2 z
vrcpph RM z,z/m E.66.m6.w0 4c /r lz fv b2 z
vrcpsh RVM x,x,x/m E.66.m6.w0 4d /r lig n2 z
vreducepd RMI x,x/m,i E.66.0f3a.w1 56 /r lx fv b8 z sae
vreducepd RMI y,y/m,i E.66.0f3a.w1 56 /r ly fv b8 z sae
vreducepd RMI z,z/m,i E.66.0f3a.w1 56 /r lz fv b8 z sae
vreduceph RMI x,x/m,i E.np.0f3a.w0 56 /r lx fv b2 z sae
vreduceph RMI y,y/m,i E.np.0f3a.w0 56 /r ly fv b2 z sae
vreduceph RMI z,z/m,i E.np.0f3a.w0 56 /r lz fv b2 z sae
vreduceps RMI x,x/m,i E.66.0f3a.w0 56 /r lx fv b4 z sae
vreduceps RMI y,y/m,i E.66.0f3a.w0 56 /r ly fv b4 z sae
vreduceps RMI z,z/m,i E.66.0f3a.w0 56 /r lz fv b4 z sae
vreducesd RVMI x,x,x/m,i E.66.0f3a.w1 57 /r lig n8 z sae
vreducesh RVMI x,x,x/m,i E.np.0f3a.w0 57 /r lig n2 z sae
vreducess RVMI x,x,x/m,i E.66.0f3a.w0 57 /r lig n4 z sae
vrndscalepd RMI x,x/m,i E.66.0f3a.w1 09 /r lx fv b8 z sae
vrndscalepd RMI y,y/m,i E.66.0f3a.w1 09 /r ly fv b8 z sae
vrndscalepd RMI z,z/m,i E.66.0f3a.w1 09 /r lz fv b8 z sae
vrndscaleph RMI x,x/m,i E.np.0f3a.w0 08 /r lx fv b2 z sae
vrndscaleph RMI y,y/m,i E.np.0f3a.w0 08 /r ly fv b2 z sae
vrndscaleph RMI z,z/m,i E.np.0f3a.w0 08 /r lz fv b2 z sae
vrndscaleps RMI x,x/m,i E.66.0f3a.w0 08 /r lx fv b4 z sae
vrndscaleps RMI y,y/m,i E.66.0f3a.w0 08 /r ly fv b4 z sae
vrndscaleps RMI z,z/m,i E.66.0f3a.w0 08 /r lz fv b4 z sae
vrndscalesd RVMI x,x,x/m,i E.66.0f3a.w1 0b /r lig n8 z sae
vrndscalesh RVMI x,x,x/m,i E.np.0f3a.w0 0a /r lig n2 z sae
vrndscaless RVMI x,x,x/m,i E.66.0f3a.w0 0a /r lig n4 z sae
vrsqrt14pd RM x,x/m E.66.0f38.w1 4e /r lx fv b8 z
vrsqrt14pd RM x,x/m E.f2.0f38.w1 4e /r lx fv b8 z
vrsqrt14pd RM x,x/m E.f3.0f38.w1 4e /r lx fv b8 z
vrsqrt14pd RM x,x/m E.np.0f38.w1 4e /r lx fv b8 z
vrsqrt14pd RM y,y/m E.66.0f38.w1 4e /r ly fv b8 z
vrsqrt14pd RM y,y/m E.f2.0f38.w1 4e /r ly fv b8 z
vrsqrt14pd RM y,y/m E.f3.0f38.w1 4e /r ly fv b8 z
vrsqrt14pd RM y,y/m E.np.0f38.w1 4e /r ly fv b8 z
vrsqrt14pd RM z,z/m E.66.0f38.w1 4e /r lz fv b8 z
vrsqrt14pd RM z,z/m E.f2.0f38.w1 4e /r lz fv b8 z
vrsqrt14pd RM z,z/m E.f3.0f38.w1 4e /r lz fv b8 z
vrsqrt14pd RM z,z/m E.np.0f38.w1 4e /r lz fv b8 z
vrsqrt14ps RM x,x/m E.66.0f38.w0 4e /r lx fv b4 z
vrsqrt14ps RM x,x/m E.f2.0f38.w0 4e /r lx fv b4 z
vrsqrt14ps RM x,x/m E.f3.0f38.w0 4e /r lx fv b4 z
vrsqrt14ps RM x,x/m E.np.0f38.w0 4e /r lx fv b4 z
vrsqrt14ps RM y,y/m E.66.0f38.w0 4e /r ly fv b4 z
vrsqrt14ps RM y,y/m E.f2.0f38.w0 4e /r ly fv b4 z
vrsqrt14ps RM y,y/m E.f3.0f38.w0 4e /r ly fv b4 z
vrsqrt14ps RM y,y/m E.np.0f38.w0 4e /r ly fv b4 z
vrsqrt14ps RM z,z/m E.66.0f38.w0 4e /r lz fv b4 z
vrsqrt14ps RM z,z/m E.f2.0f38.w0 4e /r lz fv b4 z
vrsqrt14ps RM z,z/m E.f3.0f38.w0 4e /r lz fv b4 z
vrsqrt14ps RM z,z/m E.np.0f38.w0 4e /r lz fv b4 z
vrsqrt14sd RVM x,x,x/m E.66.0f38.w1 4f /r lig n8 z
vrsqrt14ss RVM x,x,x/m E.66.0f38.w0 4f /r lig n4 z
vrsqrt28pd RM z,z/m E.66.0f38.w1 cc /r lz fv b8 z sae
vrsqrt28ps RM z,z/m E.66.0f38.w0 cc /r lz fv b4 z sae
vrsqrt28sd RVM x,x,x/m E.66.0f38.w1 cd /r lig n8 z sae
vrsqrt28ss RVM x,x,x/m E.66.0f38.w0 cd /r lig n4 z sae
vrsqrtph RM x,x/m E.66.m6.w0 4e /r lx fv b2 z
vrsqrtph RM y,y/m E.66.m6.w0 4e /r ly fv b2 z
vrsqrtph RM z,z/m E.66.m6.w0 4e /r lz fv b2 z
vrsqrtsh RVM x,x,x/m E.66.m6.w0 4f /r lig n2 z
vscalefpd RVM x,x,x/m E.66.0f38.w1 2c /r lx fv b8 z er
vscalefpd RVM y,y,y/m E.66.0f38.w1 2c /r ly fv b8 z er
vscalefpd RVM z,z,z/m E.66.0f38.w1 2c /r lz fv b8 z er
vscalefph RVM x,x,x/m E.66.m6.w0 2c /r lx fv b2 z er
vscalefph RVM y,y,y/m E.66.m6.w0 2c /r ly fv b2 z er
vscalefph RVM z,z,z/m E.66.m6.w0 2c /r lz fv b2 z er
vscalefps RVM x,x,x/m E.66.0f38.w0 2c /r lx fv b4 z er
vscalefps RVM y,y,y/m E.66.0f38.w0 2c /r ly fv b4 z er
vscalefps RVM z,z,z/m E.66.0f38.w0 2c /r lz fv b4 z er
vscalefsd RVM x,x,x/m E.66.0f38.w1 2d /r lig n8 z er
vscalefsh RVM x,x,x/m E.66.m6.w0 2d /r lig n2 z er
vscalefss RVM x,x,x/m E.66.0f38.w0 2d /r lig n4 z er
vshuff32x4 RVMI y,y,y/m,i E.66.0f3a.w0 23 /r ly fv b4 z
vshuff32x4 RVMI z,z,z/m,i E.66.0f3a.w0 23 /r lz fv b4 z
vshuff64x2 RVMI y,y,y/m,i E.66.0f3a.w1 23 /r ly fv b8 z
vshuff64x2 RVMI z,z,z/m,i E.66.0f3a.w1 23 /r lz fv b8 z
vshufi32x4 RVMI y,y,y/m,i E.66.0f3a.w0 43 /r ly fv b4 z
vshufi32x4 RVMI z,z,z/m,i E.66.0f3a.w0 43 /r lz fv b4 z
vshufi64x2 RVMI y,y,y/m,i E.66.0f3a.w1 43 /r ly fv b8 z
vshufi64x2 RVMI z,z,z/m,i E.66.0f3a.w1 43 /r lz fv b8 z
vsqrtph RM x,x/m E.np.m5.w0 51 /r lx fv b2 z er
vsqrtph RM y,y/m E.np.m5.w0 51 /r ly fv b2 z er
vsqrtph RM z,z/m E.np.m5.w0 51 /r lz fv b2 z er
vsqrtsh RVM x,x,x/m E.f3.m5.w0 51 /r lig n2 z er
vsubph RVM x,x,x/m E.np.m5.w0 5c /r lx fv b2 z er
vsubph RVM y,y,y/m E.np.m5.w0 5c /r ly fv b2 z er
vsubph RVM z,z,z/m E.np.m5.w0 5c /r lz fv b2 z er
vsubsh RVM x,x,x/m E.f3.m5.w0 5c /r lig n2 z er
vucomish RM x,x/m E.np.m5.w0 2e /r lig n2 sae
"""

VEX_FORMS = """
cmpbexadd MRV m,r32,r32 V.66.0f38.w0 e6 /r lx
cmpbexadd MRV m,r64,r64 V.66.0f38.w1 e6 /r lx
cmpbxadd MRV m,r32,r32 V.66.0f38.w0 e2 /r lx
cmpbxadd MRV m,r64,r64 V.66.0f38.w1 e2 /r lx
cmplexadd MRV m,r32,r32 V.66.0f38.w0 ee /r lx
cmplexadd MRV m,r64,r64 V.66.0f38.w1 ee /r lx
cmplxadd MRV m,r32,r32 V.66.0f38.w0 ec /r lx
cmplxadd MRV m,r64,r64 V.66.0f38.w1 ec /r lx
cmpnbexadd MRV m,r32,r32 V.66.0f38.w0 e7 /r lx
cmpnbexadd MRV m,r64,r64 V.66.0f38.w1 e7 /r lx
cmpnbxadd MRV m,r32,r32 V.66.0f38.w0 e3 /r lx
cmpnbxadd MRV m,r64,r64 V.66.0f38.w1 e3 /r lx
cmpnlexadd MRV m,r32,r32 V.66.0f38.w0 ef /r lx
cmpnlexadd MRV m,r64,r64 V.66.0f38.w1 ef /r lx
cmpnlxadd MRV m,r32,r32 V.66.0f38.w0 ed /r lx
cmpnlxadd MRV m,r64,r64 V.66.0f38.w1 ed /r lx
cmpnoxadd MRV m,r32,r32 V.66.0f38.w0 e1 /r lx
cmpnoxadd MRV m,r64,r64 V.66.0f38.w1 e1 /r lx
cmpnpxadd MRV m,r32,r32 V.66.0f38.w0 eb /r lx
cmpnpxadd MRV m,r64,r64 V.66.0f38.w1 eb /r lx
cmpnsxadd MRV m,r32,r32 V.66.0f38.w0 e9 /r lx
cmpnsxadd MRV m,r64,r64 V.66.0f38.w1 e9 /r lx
cmpnzxadd MRV m,r32,r32 V.66.0f38.w0 e5 /r lx
cmpnzxadd MRV m,r64,r64 V.66.0f38.w1 e5 /r lx
cmpoxadd MRV m,r32,r32 V.66.0f38.w0 e0 /r lx
cmpoxadd MRV m,r64,r64 V.66.0f38.w1 e0 /r lx
cmppxadd MRV m,r32,r32 V.66.0f38.w0 ea /r lx
cmppxadd MRV m,r64,r64 V.66.0f38.w1 ea /r lx
cmpsxadd MRV m,r32,r32 V.66.0f38.w0 e8 /r lx
cmpsxadd MRV m,r64,r64 V.66.0f38.w1 e8 /r lx
cmpzxadd MRV m,r32,r32 V.66.0f38.w0 e4 /r lx
cmpzxadd MRV m,r64,r64 V.66.0f38.w1 e4 /r lx
kshiftlb RMI k,k,i V.66.0f3a.w0 32 /r lx
kshiftld RMI k,k,i V.66.0f3a.w0 33 /r lx
kshiftlq RMI k,k,i V.66.0f3a.w1 33 /r lx
kshiftlw RMI k,k,i V.66.0f3a.w1 32 /r lx
kshiftrb RMI k,k,i V.66.0f3a.w0 30 /r lx
kshiftrd RMI k,k,i V.66.0f3a.w0 31 /r lx
kshiftrq RMI k,k,i V.66.0f3a.w1 31 /r lx
kshiftrw RMI k,k,i V.66.0f3a.w1 30 /r lx
vbcstnebf162ps RM x,m V.f3.0f38.w0 b1 /r lx
vbcstnebf162ps RM y,m V.f3.0f38.w0 b1 /r ly
vbcstnesh2ps RM x,m V.66.0f38.w0 b1 /r lx
vbcstnesh2ps RM y,m V.66.0f38.w0 b1 /r ly
vbroadcastf128 RM y,m V.66.0f38.w0 1a /r ly
vbroadcasti128 RM y,m V.66.0f38.w0 5a /r ly
vcvtdq2pd RM x,x/m V.f3.0f.w0 e6 /r lx
vcvtdq2pd RM y,x/m V.f3.0f.w0 e6 /r ly
vcvtneebf162ps RM x,m V.f3.0f38.w0 b0 /r lx
vcvtneebf162ps RM y,m V.f3.0f38.w0 b0 /r ly
vcvtneeph2ps RM x,m V.66.0f38.w0 b0 /r lx
vcvtneeph2ps RM y,m V.66.0f38.w0 b0 /r ly
vcvtneobf162ps RM x,m V.f2.0f38.w0 b0 /r lx
vcvtneobf162ps RM y,m V.f2.0f38.w0 b0 /r ly
vcvtneoph2ps RM x,m V.np.0f38.w0 b0 /r lx
vcvtneoph2ps RM y,m V.np.0f38.w0 b0 /r ly
vcvtpd2dq RM x,x V.f2.0f.w0 e6 /r lx
vcvtpd2dq RM x,y V.f2.0f.w0 e6 /r ly
vcvtpd2dqx RM x,m V.f2.0f.w0 e6 /r lx
vcvtpd2dqy RM x,m V.f2.0f.w0 e6 /r ly
vcvttpd2dq RM x,x V.66.0f.w0 e6 /r lx
vcvttpd2dq RM x,y V.66.0f.w0 e6 /r ly
vcvttpd2dqx RM x,m V.66.0f.w0 e6 /r lx
vcvttpd2dqy RM x,m V.66.0f.w0 e6 /r ly
vfmaddpd RVSM x,x,x,x/m V.66.0f3a.w1 69 /r lx
vfmaddpd RVSM y,y,y,y/m V.66.0f3a.w1 69 /r ly
vfmaddpd RVMS x,x,x/m,x V.66.0f3a.w0 69 /r lx
vfmaddpd RVMS y,y,y/m,y V.66.0f3a.w0 69 /r ly
vfmaddps RVSM x,x,x,x/m V.66.0f3a.w1 68 /r lx
vfmaddps RVSM y,y,y,y/m V.66.0f3a.w1 68 /r ly
vfmaddps RVMS x,x,x/m,x V.66.0f3a.w0 68 /r lx
vfmaddps RVMS y,y,y/m,y V.66.0f3a.w0 68 /r ly
vfmaddsd RVSM x,x,x,x/m V.66.0f3a.w1 6b /r lig
vfmaddsd RVMS x,x,x/m,x V.66.0f3a.w0 6b /r lig
vfmaddss RVSM x,x,x,x/m V.66.0f3a.w1 6a /r lig
vfmaddss RVMS x,x,x/m,x V.66.0f3a.w0 6a /r lig
vfmaddsubpd RVSM x,x,x,x/m V.66.0f3a.w1 5d /r lx
vfmaddsubpd RVSM y,y,y,y/m V.66.0f3a.w1 5d /r ly
vfmaddsubpd RVMS x,x,x/m,x V.66.0f3a.w0 5d /r lx
vfmaddsubpd RVMS y,y,y/m,y V.66.0f3a.w0 5d /r ly
vfmaddsubps RVSM x,x,x,x/m V.66.0f3a.w1 5c /r lx
vfmaddsubps RVSM y,y,y,y/m V.66.0f3a.w1 5c /r ly
vfmaddsubps RVMS x,x,x/m,x V.66.0f3a.w0 5c /r lx
vfmaddsubps RVMS y,y,y/m,y V.66.0f3a.w0 5c /r ly
vfmsubaddpd RVSM x,x,x,x/m V.66.0f3a.w1 5f /r lx
vfmsubaddpd RVSM y,y,y,y/m V.66.0f3a.w1 5f /r ly
vfmsubaddpd RVMS x,x,x/m,x V.66.0f3a.w0 5f /r lx
vfmsubaddpd RVMS y,y,y/m,y V.66.0f3a.w0 5f /r ly
vfmsubaddps RVSM x,x,x,x/m V.66.0f3a.w1 5e /r lx
vfmsubaddps RVSM y,y,y,y/m V.66.0f3a.w1 5e /r ly
vfmsubaddps RVMS x,x,x/m,x V.66.0f3a.w0 5e /r lx
vfmsubaddps RVMS y,y,y/m,y V.66.0f3a.w0 5e /r ly
vfmsubpd RVSM x,x,x,x/m V.66.0f3a.w1 6d /r lx
vfmsubpd RVSM y,y,y,y/m V.66.0f3a.w1 6d /r ly
vfmsubpd RVMS x,x,x/m,x V.66.0f3a.w0 6d /r lx
vfmsubpd RVMS y,y,y/m,y V.66.0f3a.w0 6d /r ly
vfmsubps RVSM x,x,x,x/m V.66.0f3a.w1 6c /r lx
vfmsubps RVSM y,y,y,y/m V.66.0f3a.w1 6c /r ly
vfmsubps RVMS x,x,x/m,x V.66.0f3a.w0 6c /r lx
vfmsubps RVMS y,y,y/m,y V.66.0f3a.w0 6c /r ly
vfmsubsd RVSM x,x,x,x/m V.66.0f3a.w1 6f /r lig
vfmsubsd RVMS x,x,x/m,x V.66.0f3a.w0 6f /r lig
vfmsubss RVSM x,x,x,x/m V.66.0f3a.w1 6e /r lig
vfmsubss RVMS x,x,x/m,x V.66.0f3a.w0 6e /r lig
vfnmaddpd RVSM x,x,x,x/m V.66.0f3a.w1 79 /r lx
vfnmaddpd RVSM y,y,y,y/m V.66.0f3a.w1 79 /r ly
vfnmaddpd RVMS x,x,x/m,x V.66.0f3a.w0 79 /r lx
vfnmaddpd RVMS y,y,y/m,y V.66.0f3a.w0 79 /r ly
vfnmaddps RVSM x,x,x,x/m V.66.0f3a.w1 78 /r lx
vfnmaddps RVSM y,y,y,y/m V.66.0f3a.w1 78 /r ly
vfnmaddps RVMS x,x,x/m,x V.66.0f3a.w0 78 /r lx
vfnmaddps RVMS y,y,y/m,y V.66.0f3a.w0 78 /r ly
vfnmaddsd RVSM x,x,x,x/m V.66.0f3a.w1 7b /r lig
vfnmaddsd RVMS x,x,x/m,x V.66.0f3a.w0 7b /r lig
vfnmaddss RVSM x,x,x,x/m V.66.0f3a.w1 7a /r lig
vfnmaddss RVMS x,x,x/m,x V.66.0f3a.w0 7a /r lig
vfnmsubpd RVSM x,x,x,x/m V.66.0f3a.w1 7d /r lx
vfnmsubpd RVSM y,y,y,y/m V.66.0f3a.w1 7d /r ly
vfnmsubpd RVMS x,x,x/m,x V.66.0f3a.w0 7d /r lx
vfnmsubpd RVMS y,y,y/m,y V.66.0f3a.w0 7d /r ly
vfnmsubps RVSM x,x,x,x/m V.66.0f3a.w1 7c /r lx
vfnmsubps RVSM y,y,y,y/m V.66.0f3a.w1 7c /r ly
vfnmsubps RVMS x,x,x/m,x V.66.0f3a.w0 7c /r lx
vfnmsubps RVMS y,y,y/m,y V.66.0f3a.w0 7c /r ly
vfnmsubsd RVSM x,x,x,x/m V.66.0f3a.w1 7f /r lig
vfnmsubsd RVMS x,x,x/m,x V.66.0f3a.w0 7f /r lig
vfnmsubss RVSM x,x,x,x/m V.66.0f3a.w1 7e /r lig
vfnmsubss RVMS x,x,x/m,x V.66.0f3a.w0 7e /r lig
vmaskmovdqu RM x,x V.66.0f.w0 f7 /r lx
vmovntdq MR m,x V.66.0f.w0 e7 /r lx
vmovntdq MR m,y V.66.0f.w0 e7 /r ly
vmovntdqa RM x,m V.66.0f38.w0 2a /r lx
vmovntdqa RM y,m V.66.0f38.w0 2a /r ly
vmovntpd MR m,x V.66.0f.w0 2b /r lx
vmovntpd MR m,y V.66.0f.w0 2b /r ly
vmovntps MR m,x V.np.0f.w0 2b /r lx
vmovntps MR m,y V.np.0f.w0 2b /r ly
vpcmpestriq RMI x,x/m,i V.66.0f3a.w1 61 /r lx
vpcmpestrmq RMI x,x/m,i V.66.0f3a.w1 60 /r lx
vpdpbssd RVM x,x,x/m V.f2.0f38.w0 50 /r lx
vpdpbssd RVM y,y,y/m V.f2.0f38.w0 50 /r ly
vpdpbssds RVM x,x,x/m V.f2.0f38.w0 51 /r lx
vpdpbssds RVM y,y,y/m V.f2.0f38.w0 51 /r ly
vpdpbsud RVM x,x,x/m V.f3.0f38.w0 50 /r lx
vpdpbsud RVM y,y,y/m V.f3.0f38.w0 50 /r ly
vpdpbsuds RVM x,x,x/m V.f3.0f38.w0 51 /r lx
vpdpbsuds RVM y,y,y/m V.f3.0f38.w0 51 /r ly
vpdpbuud RVM x,x,x/m V.np.0f38.w0 50 /r lx
vpdpbuud RVM y,y,y/m V.np.0f38.w0 50 /r ly
vpdpbuuds RVM x,x,x/m V.np.0f38.w0 51 /r lx
vpdpbuuds RVM y,y,y/m V.np.0f38.w0 51 /r ly
vpdpwsud RVM x,x,x/m V.f3.0f38.w0 d2 /r lx
vpdpwsud RVM y,y,y/m V.f3.0f38.w0 d2 /r ly
vpdpwsuds RVM x,x,x/m V.f3.0f38.w0 d3 /r lx
vpdpwsuds RVM y,y,y/m V.f3.0f38.w0 d3 /r ly
vpdpwusd RVM x,x,x/m V.66.0f38.w0 d2 /r lx
vpdpwusd RVM y,y,y/m V.66.0f38.w0 d2 /r ly
vpdpwusds RVM x,x,x/m V.66.0f38.w0 d3 /r lx
vpdpwusds RVM y,y,y/m V.66.0f38.w0 d3 /r ly
vpdpwuud RVM x,x,x/m V.np.0f38.w0 d2 /r lx
vpdpwuud RVM y,y,y/m V.np.0f38.w0 d2 /r ly
vpdpwuuds RVM x,x,x/m V.np.0f38.w0 d3 /r lx
vpdpwuuds RVM y,y,y/m V.np.0f38.w0 d3 /r ly
vpermil2pd RVMSI x,x,x/m,x,i V.66.0f3a.w0 49 /r lx
vpermil2pd RVMSI y,y,y/m,y,i V.66.0f3a.w0 49 /r ly
vpermil2pd RVSMI x,x,x,x/m,i V.66.0f3a.w1 49 /r lx
vpermil2pd RVSMI y,y,y,y/m,i V.66.0f3a.w1 49 /r ly
vpermil2ps RVMSI x,x,x/m,x,i V.66.0f3a.w0 48 /r lx
vpermil2ps RVMSI y,y,y/m,y,i V.66.0f3a.w0 48 /r ly
vpermil2ps RVSMI x,x,x,x/m,i V.66.0f3a.w1 48 /r lx
vpermil2ps RVSMI y,y,y,y/m,i V.66.0f3a.w1 48 /r ly
vpextrw MRI m,x,i V.66.0f3a.w0 15 /r lx
vpmovsxbd RM x,x/m V.66.0f38.w0 21 /r lx
vpmovsxbd RM y,x/m V.66.0f38.w0 21 /r ly
vpmovsxbq RM x,x/m V.66.0f38.w0 22 /r lx
vpmovsxbq RM y,x/m V.66.0f38.w0 22 /r ly
vpmovsxdq RM x,x/m V.66.0f38.w0 25 /r lx
vpmovsxdq RM y,x/m V.66.0f38.w0 25 /r ly
vpmovsxwd RM x,x/m V.66.0f38.w0 23 /r lx
vpmovsxwd RM y,x/m V.66.0f38.w0 23 /r ly
vpmovsxwq RM x,x/m V.66.0f38.w0 24 /r lx
vpmovsxwq RM y,x/m V.66.0f38.w0 24 /r ly
vpmovzxbd RM x,x/m V.66.0f38.w0 31 /r lx
vpmovzxbd RM y,x/m V.66.0f38.w0 31 /r ly
vpmovzxbq RM x,x/m V.66.0f38.w0 32 /r lx
vpmovzxbq RM y,x/m V.66.0f38.w0 32 /r ly
vpmovzxdq RM x,x/m V.66.0f38.w0 35 /r lx
vpmovzxdq RM y,x/m V.66.0f38.w0 35 /r ly
vpmovzxwd RM x,x/m V.66.0f38.w0 33 /r lx
vpmovzxwd RM y,x/m V.66.0f38.w0 33 /r ly
vpmovzxwq RM x,x/m V.66.0f38.w0 34 /r lx
vpmovzxwq RM y,x/m V.66.0f38.w0 34 /r ly
vsha512msg1 RM y,x V.f2.0f38.w0 cc /r ly
vsha512msg2 RM y,y V.f2.0f38.w0 cd /r ly
vsha512rnds2 RVM y,y,x V.f2.0f38.w0 cb /r ly
vsm3msg1 RVM x,x,x/m V.np.0f38.w0 da /r lx
vsm3msg2 RVM x,x,x/m V.66.0f38.w0 da /r lx
vsm3rnds2 RVMI x,x,x/m,i V.66.0f3a.w0 de /r lx
vsm4key4 RVM x,x,x/m V.f3.0f38.w0 da /r lx
vsm4key4 RVM y,y,y/m V.f3.0f38.w0 da /r ly
vsm4rnds4 RVM x,x,x/m V.f2.0f38.w0 da /r lx
vsm4rnds4 RVM y,y,y/m V.f2.0f38.w0 da /r ly
vtestpd RM x,x/m V.66.0f38.w0 0f /r lx
vtestpd RM y,y/m V.66.0f38.w0 0f /r ly
vtestps RM x,x/m V.66.0f38.w0 0e /r lx
vtestps RM y,y/m V.66.0f38.w0 0e /r ly
"""

LEGACY_FORMS = """
"""

for x in (EVEX_FORMS + VEX_FORMS + LEGACY_FORMS).split('\n'):
    if x:
        L.append(x)
