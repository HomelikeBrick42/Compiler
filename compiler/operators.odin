package compiler

import "core:strings"

UnaryOperator :: struct {
	operator_kind: TokenKind,
	operation: Instruction,
	operand_type: ^BoundType,
	result_type: ^BoundType,
}

BinaryOperator :: struct {
	operator_kind: TokenKind,
	operation: Instruction,
	left_type: ^BoundType,
	right_type: ^BoundType,
	result_type: ^BoundType,
}

equals_to_operator := map[TokenKind]TokenKind{
	.PlusEquals     = .Plus,
	.MinusEquals    = .Minus,
	.AsteriskEquals = .Asterisk,
	.SlashEquals    = .Slash,
	.PercentEquals  = .Percent,
	.Equals         = .Equals,
}

UnaryOperator_ToString :: proc(kind: TokenKind, allocator := context.allocator) -> string {
	builder: strings.Builder
	strings.init_builder(&builder, allocator)
	UnaryOperator_Print(kind, &builder)
	return strings.to_string(builder)
}

UnaryOperator_Print :: proc(kind: TokenKind, builder: ^strings.Builder) {
	#partial switch kind {
		case .Plus: {
			strings.write_string(builder, "+")
		}

		case .Minus: {
			strings.write_string(builder, "-")
		}

		case .ExclamationMark: {
			strings.write_string(builder, "!")
		}

		case: {
			assert(false, "unreachable UnaryOperator kind in PrintUnaryOperator")
		}
	}
}

BinaryOperator_ToString :: proc(kind: TokenKind, allocator := context.allocator) -> string {
	builder: strings.Builder
	strings.init_builder(&builder, allocator)
	BinaryOperator_Print(kind, &builder)
	return strings.to_string(builder)
}

BinaryOperator_Print :: proc(kind: TokenKind, builder: ^strings.Builder) {
	#partial switch kind {
		case .Plus: {
			strings.write_string(builder, "+")
		}

		case .Minus: {
			strings.write_string(builder, "-")
		}

		case .Asterisk: {
			strings.write_string(builder, "*")
		}

		case .Slash: {
			strings.write_string(builder, "/")
		}

		case .Percent: {
			strings.write_string(builder, "%")
		}

		case .EqualsEquals: {
			strings.write_string(builder, "==")
		}

		case .ExclamationMarkEquals: {
			strings.write_string(builder, "!=")
		}

		case .LessThan: {
			strings.write_string(builder, "<")
		}

		case .LessThanEquals: {
			strings.write_string(builder, "<=")
		}

		case .GreaterThan: {
			strings.write_string(builder, ">")
		}

		case .GreaterThanEquals: {
			strings.write_string(builder, ">=")
		}

		case .AmpersandAmpersand: {
			strings.write_string(builder, "&&")
		}

		case .PipePipe: {
			strings.write_string(builder, "||")
		}

		case: {
			assert(false, "unreachable assignment BiaryOperator kind in PrintBinaryOperator")
		}
	}
}
