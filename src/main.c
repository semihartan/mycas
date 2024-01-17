#include <mc.h>

#include <io/file.h>
#include <lex/lexer.h>
#include <vm/symtab.h>
#include <parse/parser.h>
#include <parse/semantic.h>  
#include <compiler/comp.h>

#include <compiler/dis.h>

#include <vm/vm.h>

#include <util.h>
#include <mcstr.h>

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>

#include <io.h>
#include <fcntl.h>

#include "res/ver.h" 

#include <gc/gc.h>  
 

static const wchar_t* g_version = MYCAS_VERSION;
 
  
int wmain(int argc, const wchar_t* argv[])
{
	/* Initialize GC, we initialize the GC at the start of the app. */
	GC_INIT(); 
	vm_start();
	
	setlocale(LC_ALL, ".UTF8");
	 
	(void)_setmode(_fileno(stdout), _O_U16TEXT);
	(void)_setmode(_fileno(stderr), _O_U16TEXT);
	(void)_setmode(_fileno(stdin), _O_U16TEXT);

	file_t* input_file = io_open_file(argv[1]);

	parser_t* parser = parser_open_from_fd(input_file);

	ast_node_t* root = parse_program(parser);

	semantic_analyze(root);

// Uncomment to print the AST.
#define PRINT_AST

// Uncomment to print the symbol table.
#define PRINT_SYMTAB
 
// Uncomment to print opcodes compiled.
#define PRINT_OPCODES

#ifdef PRINT_AST
	ast_print_tree(root);
	putwchar(L'\n');
#endif

#ifdef PRINT_SYMTAB
	symtab_dump(symtab);
	putwchar(L'\n');
#endif

	code_object_t* co = comp_program(root);

#ifdef PRINT_OPCODES
	mc_string_t asm_listing = co_disassemble(co);
	wprintf(L"%s\n", asm_listing);
#endif

	vm_execute_program(co);
	io_close_file(&input_file);
	return 0;
}