package compiler

import "core:fmt"
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

	// BoundNode_Print(bound_file, 0)

	program: [dynamic]Instruction
	EmitBytecode(bound_file, &program)

	vm := VM_Create(program[:], 1024)
	defer VM_Destroy(&vm)

	if error := VM_Run(&vm); error != nil {
		fmt.eprintf("Virtual Machine Error: '{}'\n", error.(string))
		os.exit(1)
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
						if declaration.global {
							append(program, InstStoreGlobal{ declaration.stack_location, declaration.type.size })
						} else {
							append(program, InstStoreLocal{ declaration.stack_location, declaration.type.size })
						}
					}
				}

				case ^BoundAssignment: {
					assignment := s
					name := assignment.operand.expression_kind.(^BoundName)
					if assignment.binary_operator != nil {
						if name.declaration.global {
							append(program, InstLoadGlobal{ name.declaration.stack_location, name.type.size })
						} else {
							append(program, InstLoadLocal{ name.declaration.stack_location, name.type.size })
						}
					}
					EmitBytecode(assignment.value, program)
					if assignment.binary_operator != nil {
						append(program, assignment.binary_operator.operation)
					}
					if name.declaration.global {
						append(program, InstStoreGlobal{ name.declaration.stack_location, name.type.size })
					} else {
						append(program, InstStoreLocal{ name.declaration.stack_location, name.type.size })
					}
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
					if name.declaration.global {
						append(program, InstLoadGlobal{ name.declaration.stack_location, name.type.size })
					} else {
						append(program, InstLoadLocal{ name.declaration.stack_location, name.type.size })
					}
				}

				case ^BoundInteger: {
					integer := e
					append(program, InstPushS64{ cast(i64) integer.value })
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
