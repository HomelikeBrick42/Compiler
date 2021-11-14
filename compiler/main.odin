package compiler

import "core:fmt"
import "core:strings"
import "core:os"
import "core:reflect"

SourceLoc :: struct {
	path:     string,
	source:   string,

	position: int,
	line:     int,
	column:   int,
}

Error :: struct {
	using loc: SourceLoc,
	message: string,
}

Error_MaybeAbort :: proc(error: Maybe(Error)) {
	if error != nil {
		error := error.(Error)
		fmt.eprintf("{}:{}:{}: {}\n", error.path, error.line, error.column, error.message)
		os.exit(1)
	}
}

main :: proc() {
	// TODO: Command line input
	/*
	if len(os.args) < 2 {
		fmt.eprintf("Usage: {} <file>\n", os.args[0])
		return
	}

	path := os.args[1]
	*/

	path := "test.lang"
	bytes, ok := os.read_entire_file(path)
	if !ok {
		fmt.eprintf("Unable to open file '{}'\n", path)
		os.exit(1)
	}
	defer delete(bytes)

	source := string(bytes)

	/*
	lexer := Lexer_Create(source, path)

	for {
		token, error := Lexer_NextToken(&lexer)
		if error != nil {
			error := error.(Error)
			fmt.eprintf("{}:{}:{}: {}\n", error.path, error.line, error.column, error.message)
			return
		}

		fmt.printf("{} '{}'\n", token.kind, token.source[token.position:token.position+token.length])

		if token.kind == .EndOfFile {
			break
		}
	}
	*/

	parser, error := Parser_Create(source, path)
	Error_MaybeAbort(error)

	file: ^AstFile
	file, error = Parser_ParseFile(&parser)
	Error_MaybeAbort(error)

	binder := Binder_Create()
	defer Binder_Destroy(&binder)

	bound_file: ^BoundFile
	bound_file, error = Binder_BindFile(&binder, file)
	Error_MaybeAbort(error)

	// fmt.println(BoundNode_ToString(bound_file))

	program: [dynamic]Instruction
	EmitBytecode(bound_file, &program)

	vm := VM_Create(program[:], 1024)
	defer VM_Destroy(&vm)

	if error := VM_Run(&vm); error != nil {
		fmt.eprintf("Virtual Machine Error: '{}'\n", error.(string))
		os.exit(1)
	}
}

EmitPtr :: proc(node: ^BoundNode, program: ^[dynamic]Instruction) {
	#partial switch n in node.kind {
		case ^BoundStatement: {
			statement := n
			#partial switch s in statement.statement_kind {
				case ^BoundDeclaration: {
					declaration := s
					if declaration.global {
						append(program, InstLoadGlobalPtr{ declaration.stack_location })
					} else {
						append(program, InstLoadLocalPtr{ declaration.stack_location })
					}
				}

				case: {
					assert(false, "unreachable BoundStatement default case in EmitPtr")
				}
			}
		}

		case ^BoundExpression: {
			expression := n
			#partial switch e in expression.expression_kind {
				case ^BoundName: {
					name := e
					if name.declaration.global {
						append(program, InstLoadGlobalPtr{ name.declaration.stack_location })
					} else {
						append(program, InstLoadLocalPtr{ name.declaration.stack_location })
					}
				}

				case ^BoundArrayIndex: {
					array_index := e
					EmitPtr(array_index.operand, program)
					EmitBytecode(array_index.index, program)
					append(program, InstPushPtr{ cast(uintptr) array_index.type.size })
					append(program, InstMulPtr{})
					append(program, InstAddPtr{})
				}

				case: {
					assert(false, "unreachable BoundExpression default case in EmitPtr")
				}
			}
		}

		case: {
			assert(false, "unreachable BoundNode default case in EmitPtr")
		}
	}
}

EmitBytecode :: proc(node: ^BoundNode, program: ^[dynamic]Instruction) {
	switch n in node.kind {
		case ^BoundFile: {
			file := n
			EmitBytecode(file.scope, program)
			append(program, InstExit{})
		}

		case ^BoundStatement: {
			statement := n
			switch s in statement.statement_kind {
				case ^BoundScope: {
					scope := s
					append(program, InstAllocStack{ scope.stack_size })
					for statement in scope.statements {
						EmitBytecode(statement, program)
					}
					append(program, InstPop{ scope.stack_size })
				}

				case ^BoundDeclaration: {
					declaration := s
					if declaration.value != nil {
						EmitBytecode(declaration.value, program)
						EmitPtr(declaration, program)
						append(program, InstStorePtr{ declaration.type.size })
					}
				}

				case ^BoundAssignment: {
					assignment := s
					if assignment.binary_operator != nil {
						EmitPtr(assignment.operand, program)
						append(program, InstLoadPtr{ assignment.operand.type.size })
					}
					EmitBytecode(assignment.value, program)
					if assignment.binary_operator != nil {
						append(program, assignment.binary_operator.operation)
					}
					EmitPtr(assignment.operand, program)
					append(program, InstStorePtr{ assignment.operand.type.size })
				}

				case ^BoundStatementExpression: {
					statement_expression := s
					EmitBytecode(statement_expression.expression, program)
					append(program, InstPop{ statement_expression.expression.type.size })
				}

				case ^BoundIf: {
					iff := s
					EmitBytecode(iff.condition, program)
					jump_else_location := len(program)
					append(program, InstJumpFalse{})
					EmitBytecode(iff.then_statement, program)
					jump_end_location := len(program)
					if iff.else_statement != nil {
						append(program, InstJump{})
					}
					else_jump := &program[jump_else_location].(InstJumpFalse)
					else_jump.location = len(program)
					if iff.else_statement != nil {
						EmitBytecode(iff.else_statement, program)
						end_jump := &program[jump_end_location].(InstJump)
						end_jump.location = len(program)
					}
				}

				case ^BoundWhile: {
					whilee := s
					jump_start_location := len(program)
					EmitBytecode(whilee.condition, program)
					jump_end_location := len(program)
					append(program, InstJumpFalse{})
					EmitBytecode(whilee.then_statement, program)
					append(program, InstJump{ cast(uint) jump_start_location })
					start_jump := &program[jump_end_location].(InstJumpFalse)
					start_jump.location = len(program)
				}

				case ^BoundPrint: {
					print := s
					EmitBytecode(print.expression, program)
					assert(reflect.union_variant_typeid(print.expression.type.type_kind) == ^BoundIntegerType)
					append(program, InstPrintS64{})
				}

				case: {
					assert(false, "unreachable BoundStatement default case in EmitBytecode")
				}
			}
		}

		case ^BoundExpression: {
			expression := n
			switch e in expression.expression_kind {
				case ^BoundName: {
					name := e
					EmitPtr(name, program)
					append(program, InstLoadPtr{ name.type.size })
				}

				case ^BoundInteger: {
					integer := e
					append(program, InstPushS64{ cast(i64) integer.value })
				}

				case ^BoundArrayIndex: {
					array_index := e
					EmitPtr(array_index, program)
					append(program, InstLoadPtr{ array_index.type.size })
				}

				case ^BoundTrue: {
					truee := e
					append(program, InstPushBool{ true })
				}

				case ^BoundFalse: {
					falsee := e
					append(program, InstPushBool{ false })
				}

				case ^BoundUnary: {
					unary := e
					EmitBytecode(unary.operand, program)
					append(program, unary.unary_operator.operation)
				}

				case ^BoundBinary: {
					binary := e
					EmitBytecode(binary.left, program)
					EmitBytecode(binary.right, program)
					append(program, binary.binary_operator.operation)
				}

				case: {
					assert(false, "unreachable BoundExpression default case in EmitBytecode")
				}
			}
		}

		case ^BoundType: {
			type := n
			assert(false, "unimplemented")
		}

		case: {
			assert(false, "unreachable BoundNode default case in EmitBytecode")
		}
	}
}
