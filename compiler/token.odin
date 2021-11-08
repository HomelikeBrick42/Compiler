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
	Equals,

	PlusEquals,
	MinusEquals,
	AsteriskEquals,
	SlashEquals,
	EqualsEquals,
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
