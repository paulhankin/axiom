// =============================================================================
// llvm.ph - the parts of the LLVM C API (LLVM 18) that the code generator
// uses. LLVM's opaque handles are pointers to the empty structs below; a
// NULL handle is a 0 pointer (util.nil / util.isnil).
// =============================================================================

pub struct Context {}
pub struct Module {}
pub struct Builder {}
pub struct LType {}
pub struct Value {}
pub struct BB {}
pub struct Attribute {}
pub struct Target {}
pub struct TargetMachine {}
pub struct TargetData {}
pub struct PassOptions {}
pub struct Error {}
pub struct Use {}

// Enumerations (values from the LLVM 18 C headers).
pub const IntEQ: i32 = 32;
pub const IntNE: i32 = 33;
pub const IntUGT: i32 = 34;
pub const IntUGE: i32 = 35;
pub const IntULT: i32 = 36;
pub const IntULE: i32 = 37;
pub const IntSGT: i32 = 38;
pub const IntSGE: i32 = 39;
pub const IntSLT: i32 = 40;
pub const IntSLE: i32 = 41;
pub const RealOEQ: i32 = 1;
pub const RealOGT: i32 = 2;
pub const RealOGE: i32 = 3;
pub const RealOLT: i32 = 4;
pub const RealOLE: i32 = 5;
pub const RealUNE: i32 = 14;
pub const InternalLinkage: i32 = 8;
pub const PrivateLinkage: i32 = 9;
pub const LinkOnceODRLinkage: i32 = 3;
pub const ExternalWeakLinkage: i32 = 12;
pub const WeakAnyLinkage: i32 = 5;
pub const HiddenVisibility: i32 = 1;
pub const GlobalUnnamedAddr: i32 = 2;
pub const InitialExecTLSModel: i32 = 3;
pub const SeqCst: i32 = 7;
pub const RMWXchg: i32 = 0;
pub const RMWAdd: i32 = 1;
pub const RMWSub: i32 = 2;
pub const RMWAnd: i32 = 3;
pub const RMWOr: i32 = 5;
pub const RMWXor: i32 = 6;
pub const RMWMax: i32 = 7;
pub const RMWMin: i32 = 8;
pub const RMWUMax: i32 = 9;
pub const RMWUMin: i32 = 10;
pub const RelocStatic: i32 = 1;
pub const RelocPIC: i32 = 2;
pub const CodeModelDefault: i32 = 0;
pub const AssemblyFile: i32 = 0;
pub const ObjectFile: i32 = 1;
pub const ReturnStatusAction: i32 = 2;
pub const FunctionIndex: u32 = 0xFFFF_FFFF;
pub const ReturnIndex: u32 = 0;

pub extern "C" {
    // Contexts, modules, types
    fn LLVMContextCreate() *mut Context;
    fn LLVMModuleCreateWithNameInContext(name: *u8, c: *mut Context) *mut Module;
    fn LLVMCreateBuilderInContext(c: *mut Context) *mut Builder;
    fn LLVMInt1TypeInContext(c: *mut Context) *mut LType;
    fn LLVMInt8TypeInContext(c: *mut Context) *mut LType;
    fn LLVMInt16TypeInContext(c: *mut Context) *mut LType;
    fn LLVMInt32TypeInContext(c: *mut Context) *mut LType;
    fn LLVMInt64TypeInContext(c: *mut Context) *mut LType;
    fn LLVMIntTypeInContext(c: *mut Context, bits: u32) *mut LType;
    fn LLVMPointerTypeInContext(c: *mut Context, addrspace: u32) *mut LType;
    fn LLVMVoidTypeInContext(c: *mut Context) *mut LType;
    fn LLVMFloatTypeInContext(c: *mut Context) *mut LType;
    fn LLVMDoubleTypeInContext(c: *mut Context) *mut LType;
    fn LLVMVectorType(elem: *mut LType, n: u32) *mut LType;
    fn LLVMArrayType2(elem: *mut LType, n: u64) *mut LType;
    fn LLVMStructTypeInContext(c: *mut Context, elems: *mut *mut LType, n: u32, packed: i32) *mut LType;
    fn LLVMStructCreateNamed(c: *mut Context, name: *u8) *mut LType;
    fn LLVMStructSetBody(t: *mut LType, elems: *mut *mut LType, n: u32, packed: i32) void;
    fn LLVMStructGetTypeAtIndex(t: *mut LType, i: u32) *mut LType;
    fn LLVMCountStructElementTypes(t: *mut LType) u32;
    fn LLVMFunctionType(ret: *mut LType, params: *mut *mut LType, n: u32, vararg: i32) *mut LType;
    fn LLVMCountParamTypes(t: *mut LType) u32;
    fn LLVMGetParamTypes(t: *mut LType, dest: *mut *mut LType) void;
    fn LLVMGetReturnType(t: *mut LType) *mut LType;
    fn LLVMGetIntTypeWidth(t: *mut LType) u32;
    fn LLVMGetElementType(t: *mut LType) *mut LType;
    fn LLVMGetVectorSize(t: *mut LType) u32;
    fn LLVMTypeOf(v: *mut Value) *mut LType;
    fn LLVMABISizeOfType(td: *mut TargetData, t: *mut LType) u64;

    // Constants
    fn LLVMConstInt(t: *mut LType, v: u64, sign_extend: i32) *mut Value;
    fn LLVMConstIntOfArbitraryPrecision(t: *mut LType, nwords: u32, words: *u64) *mut Value;
    fn LLVMConstReal(t: *mut LType, v: f64) *mut Value;
    fn LLVMConstNull(t: *mut LType) *mut Value;
    fn LLVMConstAllOnes(t: *mut LType) *mut Value;
    fn LLVMGetUndef(t: *mut LType) *mut Value;
    fn LLVMGetPoison(t: *mut LType) *mut Value;
    fn LLVMConstStringInContext(c: *mut Context, s: *u8, n: u32, dont_null_terminate: i32) *mut Value;
    fn LLVMConstStructInContext(c: *mut Context, vals: *mut *mut Value, n: u32, packed: i32) *mut Value;
    fn LLVMConstNamedStruct(t: *mut LType, vals: *mut *mut Value, n: u32) *mut Value;
    fn LLVMConstArray2(t: *mut LType, vals: *mut *mut Value, n: u64) *mut Value;
    fn LLVMConstVector(vals: *mut *mut Value, n: u32) *mut Value;
    fn LLVMConstIntToPtr(v: *mut Value, t: *mut LType) *mut Value;
    fn LLVMConstIntGetZExtValue(v: *mut Value) u64;
    fn LLVMIsAConstantInt(v: *mut Value) *mut Value;
    fn LLVMIsConstant(v: *mut Value) i32;
    fn LLVMIsNull(v: *mut Value) i32;

    // Globals and functions
    fn LLVMAddGlobal(m: *mut Module, t: *mut LType, name: *u8) *mut Value;
    fn LLVMAddGlobalInAddressSpace(m: *mut Module, t: *mut LType, name: *u8, addrspace: u32) *mut Value;
    fn LLVMGetNamedGlobal(m: *mut Module, name: *u8) *mut Value;
    fn LLVMSetInitializer(g: *mut Value, v: *mut Value) void;
    fn LLVMSetLinkage(g: *mut Value, l: i32) void;
    fn LLVMSetGlobalConstant(g: *mut Value, c: i32) void;
    fn LLVMSetAlignment(v: *mut Value, a: u32) void;
    fn LLVMSetUnnamedAddress(g: *mut Value, a: i32) void;
    fn LLVMSetVisibility(g: *mut Value, v: i32) void;
    fn LLVMSetThreadLocal(g: *mut Value, b: i32) void;
    fn LLVMSetThreadLocalMode(g: *mut Value, m: i32) void;
    fn LLVMAddFunction(m: *mut Module, name: *u8, t: *mut LType) *mut Value;
    fn LLVMGetNamedFunction(m: *mut Module, name: *u8) *mut Value;
    fn LLVMGetParam(f: *mut Value, i: u32) *mut Value;
    fn LLVMAddAttributeAtIndex(f: *mut Value, idx: u32, a: *mut Attribute) void;
    fn LLVMAddCallSiteAttribute(c: *mut Value, idx: u32, a: *mut Attribute) void;
    fn LLVMCreateEnumAttribute(c: *mut Context, kind: u32, v: u64) *mut Attribute;
    fn LLVMCreateStringAttribute(c: *mut Context, k: *u8, klen: u32, v: *u8, vlen: u32) *mut Attribute;
    fn LLVMGetEnumAttributeKindForName(name: *u8, n: usize) u32;
    fn LLVMLookupIntrinsicID(name: *u8, n: usize) u32;
    fn LLVMGetIntrinsicDeclaration(m: *mut Module, id: u32, tys: *mut *mut LType, n: usize) *mut Value;
    fn LLVMIntrinsicGetType(c: *mut Context, id: u32, tys: *mut *mut LType, n: usize) *mut LType;

    // Basic blocks
    fn LLVMAppendBasicBlockInContext(c: *mut Context, f: *mut Value, name: *u8) *mut BB;
    fn LLVMGetInsertBlock(b: *mut Builder) *mut BB;
    fn LLVMGetBasicBlockTerminator(bb: *mut BB) *mut Value;
    fn LLVMGetNextBasicBlock(bb: *mut BB) *mut BB;
    fn LLVMDeleteBasicBlock(bb: *mut BB) void;
    fn LLVMBasicBlockAsValue(bb: *mut BB) *mut Value;
    fn LLVMGetFirstUse(v: *mut Value) *mut Use;
    fn LLVMPositionBuilderAtEnd(b: *mut Builder, bb: *mut BB) void;
    fn LLVMPositionBuilderBefore(b: *mut Builder, i: *mut Value) void;

    // Instructions
    fn LLVMBuildRetVoid(b: *mut Builder) *mut Value;
    fn LLVMBuildRet(b: *mut Builder, v: *mut Value) *mut Value;
    fn LLVMBuildBr(b: *mut Builder, dest: *mut BB) *mut Value;
    fn LLVMBuildCondBr(b: *mut Builder, c: *mut Value, t: *mut BB, e: *mut BB) *mut Value;
    fn LLVMBuildSwitch(b: *mut Builder, v: *mut Value, dflt: *mut BB, n: u32) *mut Value;
    fn LLVMAddCase(sw: *mut Value, v: *mut Value, dest: *mut BB) void;
    fn LLVMBuildUnreachable(b: *mut Builder) *mut Value;
    fn LLVMBuildAdd(b: *mut Builder, l: *mut Value, r: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildSub(b: *mut Builder, l: *mut Value, r: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildMul(b: *mut Builder, l: *mut Value, r: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildUDiv(b: *mut Builder, l: *mut Value, r: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildSDiv(b: *mut Builder, l: *mut Value, r: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildURem(b: *mut Builder, l: *mut Value, r: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildSRem(b: *mut Builder, l: *mut Value, r: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildFAdd(b: *mut Builder, l: *mut Value, r: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildFSub(b: *mut Builder, l: *mut Value, r: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildFMul(b: *mut Builder, l: *mut Value, r: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildFDiv(b: *mut Builder, l: *mut Value, r: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildFRem(b: *mut Builder, l: *mut Value, r: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildShl(b: *mut Builder, l: *mut Value, r: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildLShr(b: *mut Builder, l: *mut Value, r: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildAShr(b: *mut Builder, l: *mut Value, r: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildAnd(b: *mut Builder, l: *mut Value, r: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildOr(b: *mut Builder, l: *mut Value, r: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildXor(b: *mut Builder, l: *mut Value, r: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildNeg(b: *mut Builder, v: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildFNeg(b: *mut Builder, v: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildNot(b: *mut Builder, v: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildICmp(b: *mut Builder, p: i32, l: *mut Value, r: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildFCmp(b: *mut Builder, p: i32, l: *mut Value, r: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildIsNull(b: *mut Builder, v: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildSelect(b: *mut Builder, c: *mut Value, t: *mut Value, e: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildPhi(b: *mut Builder, t: *mut LType, name: *u8) *mut Value;
    fn LLVMAddIncoming(phi: *mut Value, vals: *mut *mut Value, bbs: *mut *mut BB, n: u32) void;
    fn LLVMBuildAlloca(b: *mut Builder, t: *mut LType, name: *u8) *mut Value;
    fn LLVMBuildLoad2(b: *mut Builder, t: *mut LType, p: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildStore(b: *mut Builder, v: *mut Value, p: *mut Value) *mut Value;
    fn LLVMBuildGEP2(b: *mut Builder, t: *mut LType, p: *mut Value, idx: *mut *mut Value, n: u32, name: *u8) *mut Value;
    fn LLVMBuildStructGEP2(b: *mut Builder, t: *mut LType, p: *mut Value, i: u32, name: *u8) *mut Value;
    fn LLVMBuildExtractValue(b: *mut Builder, agg: *mut Value, i: u32, name: *u8) *mut Value;
    fn LLVMBuildInsertValue(b: *mut Builder, agg: *mut Value, v: *mut Value, i: u32, name: *u8) *mut Value;
    fn LLVMBuildExtractElement(b: *mut Builder, v: *mut Value, i: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildInsertElement(b: *mut Builder, v: *mut Value, x: *mut Value, i: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildShuffleVector(b: *mut Builder, v1: *mut Value, v2: *mut Value, mask: *mut Value, name: *u8) *mut Value;
    fn LLVMBuildTrunc(b: *mut Builder, v: *mut Value, t: *mut LType, name: *u8) *mut Value;
    fn LLVMBuildZExt(b: *mut Builder, v: *mut Value, t: *mut LType, name: *u8) *mut Value;
    fn LLVMBuildSExt(b: *mut Builder, v: *mut Value, t: *mut LType, name: *u8) *mut Value;
    fn LLVMBuildFPExt(b: *mut Builder, v: *mut Value, t: *mut LType, name: *u8) *mut Value;
    fn LLVMBuildFPTrunc(b: *mut Builder, v: *mut Value, t: *mut LType, name: *u8) *mut Value;
    fn LLVMBuildSIToFP(b: *mut Builder, v: *mut Value, t: *mut LType, name: *u8) *mut Value;
    fn LLVMBuildUIToFP(b: *mut Builder, v: *mut Value, t: *mut LType, name: *u8) *mut Value;
    fn LLVMBuildPtrToInt(b: *mut Builder, v: *mut Value, t: *mut LType, name: *u8) *mut Value;
    fn LLVMBuildIntToPtr(b: *mut Builder, v: *mut Value, t: *mut LType, name: *u8) *mut Value;
    fn LLVMBuildBitCast(b: *mut Builder, v: *mut Value, t: *mut LType, name: *u8) *mut Value;
    fn LLVMBuildCall2(b: *mut Builder, t: *mut LType, f: *mut Value, args: *mut *mut Value, n: u32, name: *u8) *mut Value;
    fn LLVMBuildMemMove(b: *mut Builder, dst: *mut Value, da: u32, src: *mut Value, sa: u32, size: *mut Value) *mut Value;
    fn LLVMBuildMemSet(b: *mut Builder, p: *mut Value, v: *mut Value, len: *mut Value, align: u32) *mut Value;
    fn LLVMBuildFence(b: *mut Builder, ordering: i32, single_thread: i32, name: *u8) *mut Value;
    fn LLVMBuildAtomicRMW(b: *mut Builder, op: i32, p: *mut Value, v: *mut Value, ordering: i32, single_thread: i32) *mut Value;
    fn LLVMBuildAtomicCmpXchg(b: *mut Builder, p: *mut Value, cmp: *mut Value, new: *mut Value, so: i32, fo: i32,
                              single_thread: i32) *mut Value;
    fn LLVMSetOrdering(v: *mut Value, o: i32) void;
    fn LLVMSetVolatile(v: *mut Value, b: i32) void;

    // Targets and output
    fn LLVMInitializeWebAssemblyTargetInfo() void;
    fn LLVMInitializeWebAssemblyTarget() void;
    fn LLVMInitializeWebAssemblyTargetMC() void;
    fn LLVMInitializeWebAssemblyAsmPrinter() void;
    fn LLVMInitializeX86TargetInfo() void;
    fn LLVMInitializeX86Target() void;
    fn LLVMInitializeX86TargetMC() void;
    fn LLVMInitializeX86AsmPrinter() void;
    fn LLVMInitializeAArch64TargetInfo() void;
    fn LLVMInitializeAArch64Target() void;
    fn LLVMInitializeAArch64TargetMC() void;
    fn LLVMInitializeAArch64AsmPrinter() void;
    fn LLVMGetTargetFromTriple(triple: *u8, t: *mut *mut Target, err: *mut *mut u8) i32;
    fn LLVMGetHostCPUName() *mut u8;
    fn LLVMGetDefaultTargetTriple() *mut u8;
    fn LLVMCreateTargetMachine(t: *mut Target, triple: *u8, cpu: *u8, features: *u8, level: i32, reloc: i32,
                               code_model: i32) *mut TargetMachine;
    fn LLVMCreateTargetDataLayout(tm: *mut TargetMachine) *mut TargetData;
    fn LLVMSetTarget(m: *mut Module, triple: *u8) void;
    fn LLVMSetModuleDataLayout(m: *mut Module, td: *mut TargetData) void;
    fn LLVMVerifyModule(m: *mut Module, action: i32, msg: *mut *mut u8) i32;
    fn LLVMDisposeMessage(msg: *mut u8) void;
    fn LLVMCreatePassBuilderOptions() *mut PassOptions;
    fn LLVMDisposePassBuilderOptions(o: *mut PassOptions) void;
    fn LLVMRunPasses(m: *mut Module, passes: *u8, tm: *mut TargetMachine, o: *mut PassOptions) *mut Error;
    fn LLVMGetErrorMessage(e: *mut Error) *mut u8;
    fn LLVMDisposeErrorMessage(msg: *mut u8) void;
    fn LLVMPrintModuleToFile(m: *mut Module, path: *u8, err: *mut *mut u8) i32;
    fn LLVMTargetMachineEmitToFile(tm: *mut TargetMachine, m: *mut Module, path: *u8, kind: i32, err: *mut *mut u8) i32;
}
