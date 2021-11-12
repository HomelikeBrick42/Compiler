package compiler

TokenKind :: enum {
	Invalid,

	EndOfFile,
	Name,
	Integer,

	PrintKeyword, // This is temporary

	OpenParenthesis,
	CloseParenthesis,
	OpenBrace,
	CloseBrace,
	Colon,
	Semicolon,

	Plus,
	Minus,
	Asterisk,
	Slash,
	Percent,
	Equals,
	ExclamationMark,

	PlusEquals,
	MinusEquals,
	AsteriskEquals,
	SlashEquals,
	PercentEquals,
	EqualsEquals,
	ExclamationMarkEquals,
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
