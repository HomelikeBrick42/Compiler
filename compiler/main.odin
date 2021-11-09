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

	BoundNode_Print(bound_file, 0)
}
