package compiler

TokenKind :: enum {
	Invalid,

	EndOfFile,
	Name,
	Integer,

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
