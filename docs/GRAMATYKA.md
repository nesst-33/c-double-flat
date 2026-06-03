
```EBNF
program         = { [ statement ], newline }, EOT ;

statement   = if_stmt
			| while_stmt
			| scope
			| var_or_func_decl
			| void_func_decl
			| id_arr_func_call

if_stmt = "if", condition, [ newline ], scope, [ else_body ];

condition = "(", expression, ")";

else_body = "else", [ newline ], scope; 

while_stmt = "while", condition, [ newline ], scope;

var_or_func_decl    = type, identifier, 
					( [ var_decl_assign ] | func_declaration );
					
var_decl_assign = "=", expression;

type       = [ "const" ], [ "arr", { "arr" } ], base_type;

void_func_decl = "void", identifier, func_declaration;

func_declaration = "(", [ parameters ], ")", [ newline ], scope;

scope   = "{", { [ scoped_stmt ], newline }, "}";

parameters = type, identifier, { ",", type, identifier };

scoped_stmt = var_decl
			| scope
			| if_stmt
			| while_stmt
			| return_stmt
			| id_arr_func_call
			
return_stmt = "return", [ expression ];

var_decl = type, identifier, [ var_decl_assign ];

id_arr_func_call    = identifier, 
					( func_call 
					| assign);

func_call = "(", [ arguments ], ")";
arguments = expression, { ",", expression };

assign = [ array_idx ], assign_op, expression;
array_idx = "[", expression, "]", { "[", expression, "]" };

expression = logical_and, { "or", logical_and };
logical_and = equality, { "and", equality };
equality = relational, [ ( "==" | "!=" ), relational ];
relational = arr_ops, [ ( "<" | ">" | "<=" | ">=" ), arr_ops ];

arr_ops     = additive, { ( "~" | "&" | ":" | "<<" | ">>" ), 
			  additive };

additive    = multipl, { ( "+" | "-" ), multipl };
multipl     = unary, { ( "*" | "/" | "%" ), unary };
unary       = { "+" | "-" | "not" }, postfix;

postfix     = type_cast, { "!" | ( "as", base_type ) }; 

arr_expr    = subject, { "[", expression, "]" }

subject     = id_or_func_call
			| literal
			| nested_expr;
			
id_or_func_call = identifier, [ func_call ]

literal     = int_lit
			| flp_lit
			| str_lit
			| bool_lit
			| arr_lit

nested_expr = ( "(", expression, ")" ); 

identifier = ( letter | underscore ) , { letter | digit | underscore };

assign_op   = "="
			| "+="
			| "-="
			| "*="
			| "/="
			| "%="
			| "~=" ;

letter     = [A-Za-z];
digit      = [0-9];
underscore = "_";
newline    = "\n";

escape_sequence = "\\" , ( "\"" | "'" | "\\" | "n" | "t" | "r" ) ;

bool_lit   = "true" | "false" ;
int_lit    = digit , { digit } ;

(* flp może być w 3 formach: 3.14, 3., .14 *)
flp_lit    = ( int_lit, ".", [ int_lit ] )
		   | ( ".", int_lit )

str_lit    = ('"' , 
				{ character_except_double_quote | escape_sequence } ,
				'"' )
		   | ( "'" , 
				{ character_except_single_quote | escape_sequence } ,
			    "'" ) ;
				
character_except_single_quote = [^'\]
character_except_double_quote = [^"\]

arr_lit    = "[" , [ expression, { "," , expression} ] , "]" ;

base_type  = "int" | "flp" | "str" | "bool" ;

```