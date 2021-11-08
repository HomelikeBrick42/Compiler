package compiler

import "core:fmt"
import "core:os"

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
		fmt.eprintf("Unable to open file {}\n", path)
		return
	}

	source := string(bytes)
	lexer := Lexer_Create(source, path)

	for {
		token, error := Lexer_NextToken(&lexer)
		if error != nil {
			error := error.(Error)
			fmt.eprintf("{}:{}:{}: {}\n", error.path, error.line, error.column, error.message)
			return
		}

		fmt.println(token.kind)

		if token.kind == .EndOfFile {
			break
		}
	}
}
