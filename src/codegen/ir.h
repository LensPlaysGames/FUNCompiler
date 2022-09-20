#ifndef IR_H
#define IR_H

#include <codegen/codegen_forward.h>

enum IRInstructionType {
  IR_INSTRUCTION_ADD,
  IR_INSTRUCTION_SUB,
  IR_INSTRUCTION_MUL,
  IR_INSTRUCTION_DIV,
  IR_INSTRUCTION_MOD,
  IR_INSTRUCTION_SHL,
  IR_INSTRUCTION_SAR,

  IR_INSTRUCTION_ALLOCA,
  IR_INSTRUCTION_CALL,
  IR_INSTRUCTION_COMMENT,
  IR_INSTRUCTION_COMPARISON,
  IR_INSTRUCTION_BRANCH,
  IR_INSTRUCTION_BRANCH_IF,
  IR_INSTRUCTION_IMMEDIATE,
  IR_INSTRUCTION_PHI,
  IR_INSTRUCTION_RETURN,
  IR_INSTRUCTION_FUNCTION_REF,

  IR_INSTRUCTION_GLOBAL_REF,
  IR_INSTRUCTION_GLOBAL_VAL,
  IR_INSTRUCTION_STORE_GLOBAL,
  IR_INSTRUCTION_LOCAL_REF,
  IR_INSTRUCTION_LOCAL_VAL,
  IR_INSTRUCTION_STORE_LOCAL,
  IR_INSTRUCTION_STORE,
  IR_INSTRUCTION_PARAM_REF,

  // Used by the backend. Do not use these directly.
  // FIXME(Sirraide): Should these be in a separate enum?
  IR_INSTRUCTION_COPY,
  IR_INSTRUCTION_COPY_REGISTER,
  IR_INSTRUCTION_ACQUIRE,
  IR_INSTRUCTION_RELEASE,
  IR_INSTRUCTION_ADD_TWO_ADDRESS,
  IR_INSTRUCTION_SUB_TWO_ADDRESS,
  IR_INSTRUCTION_MUL_TWO_ADDRESS,
  IR_INSTRUCTION_DIV_ONE_ADDRESS,
  IR_INSTRUCTION_SHL_TWO_ADDRESS,
  IR_INSTRUCTION_SAR_TWO_ADDRESS,

  IR_INSTRUCTION_COUNT
};

struct Function {
  char *name;
  size_t value_count;
  BasicBlock *entry;
  BasicBlock *last;
  Function *next;
  Value *return_value;
};

struct BasicBlock {
  Function *parent;
  BasicBlock *next;
  BasicBlock *prev;
  /// Linked list of values.
  Value *values;
  /// The last value in the linked list.
  Value *end;
  size_t id;
  char closed;
};

typedef struct FunctionCallArg {
  Value *value;
  struct FunctionCallArg *next;
} FunctionCallArg;

typedef struct FunctionCall {
  enum FunctionCallType {
    FUNCTION_CALL_TYPE_INTERNAL,
    FUNCTION_CALL_TYPE_EXTERNAL
  } type;
  union {
    Value *callee;
    const char *external_callee;
  };
  FunctionCallArg *args;
  /// Architecture-specific data.
  void *arch_call_data;
} FunctionCall;

typedef struct PHINodeEntry {
  BasicBlock *block;
  Value *value;
  struct PHINodeEntry *next;
} PHINodeEntry;

typedef struct Variable {
  long long int local_offset;
  size_t size;
} Variable;

typedef struct CondBranch {
  Value *condition;
  BasicBlock *true_branch;
  BasicBlock *false_branch;
} CondBranch;

typedef struct Comparison {
  enum ComparisonType type;
  Value *lhs;
  Value *rhs;
} Comparison;

typedef struct ParamRef {
  Function *func;
  size_t index;
} ParamRef;

typedef struct GlobalStore {
  const char *name;
  Value *value;
} GlobalStore;

/// FIXME(Sirraide): This is, without a doubt, one of the most abominable
///   structs I have ever ‘designed’. There has to be a better way of doing
///   this. Perhaps splitting it into separate structs would help.
struct Value {
  enum IRInstructionType type;
  BasicBlock *parent;
  size_t id;

  Value *next;
  Value *prev;

  union {
    char *comment_value;
    const char *global_name;
    PHINodeEntry *phi_entries;
    BasicBlock *branch_target;
    Function *function_ref;
    Value *local_ref;
    Value *operand;
    long long int immediate;
    unsigned reg;

    struct {
      Value *lhs;
      Value *rhs;
    };

    struct {
      Value *left;
      unsigned right;
    };

    FunctionCall call_value;
    Variable variable_value;
    CondBranch cond_branch_value;
    Comparison comparison;
    ParamRef param_ref;
    GlobalStore global_store;
  };

  /// Used by the backend.
  char emitted;
  char unused;
};

/// Create an external call by name.
Value *codegen_create_external_call(CodegenContext *ctx, const char *name);

/// Create an internal call.
Value *codegen_create_internal_call(CodegenContext *ctx, Value *callee);

/// Add an argument to a function call.
void codegen_add_function_arg(CodegenContext *ctx, Value *call, Value *arg);

/// Create a new function. `name` is set to an automatically generated name
/// if it's NULL or if `*name` is NULL.
Function *codegen_function_create(CodegenContext *ctx, const char **name);

/// Get a reference to a function
Value *codegen_function_ref(CodegenContext *ctx, Function  *function);

/// Create a basic block without attaching it.
BasicBlock *codegen_basic_block_create_detached(CodegenContext *ctx);

/// Create a basic block and attach it to the current function
BasicBlock *codegen_basic_block_create(CodegenContext *ctx);

/// Attach a block to the current function and set it as the insert point.
void codegen_basic_block_attach(CodegenContext *ctx, BasicBlock *block);

/// Load the address of global variable into a newly allocated value and return it.
Value *codegen_load_global_address(CodegenContext *ctx, const char *name);

/// Load the address of local variable into a newly allocated value and return it.
Value *codegen_load_local_address(CodegenContext *ctx, Value *address);

/// Load the value of global variable into a newly allocated value and return it.
Value *codegen_load_global(CodegenContext *ctx, const char  *name);

/// Load the value of local variable into a newly allocated value and return it.
Value *codegen_load_local(CodegenContext *ctx, Value *source);

/// Store a global variable.
void codegen_store_global(CodegenContext *ctx, Value *source, const char  *name);

/// Store a local variable.
void codegen_store_local(CodegenContext *ctx, Value *source, Value *dest);

/// Store data in the memory pointed to by the given address.
void codegen_store(CodegenContext *ctx, Value *data, Value *address);

/// Branch to true_block if value is true, and to false_block otherwise.
void codegen_branch_if(CodegenContext *ctx, Value *value, BasicBlock *true_block, BasicBlock *false_block);

/// Branch to a label.
void codegen_branch(CodegenContext *ctx, BasicBlock *block);

/// Load an immediate value.
Value *codegen_load_immediate(CodegenContext *ctx, long long int immediate);

/// Generate a comparison between two values.
Value *codegen_comparison(CodegenContext *ctx, enum ComparisonType type, Value *lhs, Value *rhs);

/// Add two values together.
Value *codegen_add(CodegenContext *ctx, Value *lhs, Value *rhs);

/// Subtract rhs from lhs.
Value *codegen_subtract(CodegenContext *ctx, Value *lhs, Value *rhs);

/// Multiply two values together.
Value *codegen_multiply(CodegenContext *ctx, Value *lhs, Value *rhs);

/// Divide lhs by rhs.
Value *codegen_divide(CodegenContext *ctx, Value *lhs, Value *rhs);

/// Modulo lhs by rhs.
Value *codegen_modulo(CodegenContext *ctx, Value *lhs, Value *rhs);

/// Shift lhs to the left by rhs.
Value *codegen_shift_left(CodegenContext *ctx, Value *lhs, Value *rhs);

/// Shift lhs to the right by rhs (arithmetic).
Value *codegen_shift_right_arithmetic(CodegenContext *ctx, Value *lhs, Value *rhs);

/// Allocate space on the stack.
Value *codegen_alloca(CodegenContext *ctx, long long int size);

/// Bind a function parameter.
Value *codegen_bind_function_parameter(CodegenContext *ctx, Function *function, size_t param_index);

/// Set the return value of a function.
void codegen_set_return_value(CodegenContext *ctx, Function *function, Value *value);

/// Create a return instruction.
void codegen_return(CodegenContext *context);

/// Set the entry point of the program.
void codegen_entry_point(CodegenContext *ctx, Function *function);

/// Create an empty phi node. This is used to merge control flow
///
/// A phi node is used to merge values from different control flow paths
/// into a single value. This is a very common operation in an SSA IR
/// and is used here to abstract away operations such as allocating an
/// empty register or copying values from one register to another.
///
/// The reasons why this exists are
///   - to abstract away the details of how values are allocated and moved;
///   - because not all architectures have a notion of registers;
///   - because they're a generally useful and widely used concept.
///
/// This API is used as follows:
/// ```c
/// Value *phi = codegen_phi_create(cg_context);
/// codegen_phi_add(cg_context, phi, block1, value1);
/// codegen_phi_add(cg_context, phi, block2, value2);
/// ```
Value *codegen_phi_create(CodegenContext *ctx);

/// Add a value to a phi node.
void codegen_phi_add(CodegenContext *ctx, Value *phi, BasicBlock *block, Value *value);

/// Create a comment.
void codegen_vcomment(CodegenContext *ctx, const char  *fmt, va_list ap);

/// Create a comment.
/// Backends should implement `codegen_vcomment_*()` instead.
FORMAT(printf, 2, 3)
void codegen_comment(CodegenContext *ctx, const char *fmt, ...);

/// Create a comment if verbose mode is enabled.
/// Backends should implement `codegen_vcomment_verbose_*()` instead.
FORMAT(printf, 2, 3)
void codegen_comment_verbose(CodegenContext *ctx, const char *fmt, ...);

/// Dump the intermediate representation to stdout.
void codegen_dump_ir(CodegenContext *context);

#endif // IR_H