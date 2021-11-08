package compiler

UnaryOperator :: struct {
	operator_kind: TokenKind,
	operand_type: ^BoundType,
	result_type: ^BoundType,
}

BinaryOperator :: struct {
	operator_kind: TokenKind,
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
