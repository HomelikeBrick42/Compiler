package compiler

TokenKind :: enum {
	Invalid,

	EndOfFile,
	Name,
	Integer,

	PrintKeyword, // This is temporary
	IfKeyword,
	ElseKeyword,
	DoKeyword,
	TrueKeyword,
	FalseKeyword,
	WhileKeyword,
	SizeOfKeyword,
	CastKeyword,

	OpenParenthesis,
	CloseParenthesis,
	OpenBrace,
	CloseBrace,
	OpenBracket,
	CloseBracket,
	Colon,
	Semicolon,
	Comma,

	RightArrow,

	Plus,
	Minus,
	Asterisk,
	Slash,
	Percent,
	Equals,
	ExclamationMark,
	LessThan,
	GreaterThan,
	AmpersandAmpersand,
	PipePipe,

	PlusEquals,
	MinusEquals,
	AsteriskEquals,
	SlashEquals,
	PercentEquals,
	EqualsEquals,
	ExclamationMarkEquals,
	LessThanEquals,
	GreaterThanEquals,
}

Token :: struct {
	kind: TokenKind,

	using loc: SourceLoc,
	length:    int,

	data: union {
		u64,
		string,
	},
}
