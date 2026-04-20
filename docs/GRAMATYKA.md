
## GRAMATYKA

### Znaki terminalne i literały

```EBNF
letter     = [A-Za-z];
digit      = [0-9];
underscore = "_";
newline    = "\n";

escape_sequence = "\\" , ( "\"" | "'" | "\\" | "n" | "t" | "r" ) ;

character_except_single_quote = [^']
character_except_double_quote = [^"]

bool_lit   = "true" | "false" ;
int_lit    = digit , { [ "'" ] , digit} ;

(* flp może być w 3 formach: 3.14, 3., .14 *)
flp_lit    = (
				( digit, 
					{ [ "'" ] , digit , "." , { [ "'" ] , digit } )
				| ( "." , digit , { [ "'" ] , digit } ) 
				| ( digit , { [ "'" ] , digit } , "." ) 
             ) ;

str_lit    = ('"' , 
				{ character_except_double_quote | escape_sequence } ,
				'"' )
		   | ( "'" , 
				{ character_except_single_quote | escape_sequence } ,
			    "'" ) ;
				
arr_lit    = "[" , [ expression, { "," , expression } ] , "]" ;

```

### Typy i zmienne

```EBNF
base_type  = "int" | "flp" | "str" | "bool" ;
type       = [ "const" ] , ( base_type | ( { "arr" } , base_type ) );
identifier = letter , { letter | digit | underscore } ;

var_decl   = type , identifier , [ "=" , expression ] ;
```

### Wyrażenia

```EBNF
assign_op   = "="
			| "+="
			| "-="
			| "*="
			| "/="
			| "%="
			| "~=" ;
			
expression  = logical_or , [ assign_op , expression ] ;

logical_or  = logical_and , { "or" , logical_and } ;
logical_and = equality , { "and" , equality } ;
equality    = relational , [ ( "==" | "!=" ) , relational ] ; 
relational  = arr_ops , [ ( "<" | ">" | "<=" | ">=" ) , arr_ops ] ;

arr_ops     = additive , { ( "~" | "&" | ":" | "<<" | ">>" ) , 
			  additive} ;

additive    = multipl , { ( "+" | "-" ) , multipl } ;
multipl     = unary, { ( "*" | "/" | "%" ) , unary } ;

unary       = [ "+" | "-" | "not" ] , postfix ;

(* UWAGA: jestem świadomy, że po tym jak dodałem ["const"] do type, gramatycznie poprawnym będzie napisanie czegoś w stylu "5" as const int, ale semantycznie const będzie ignorowany, ponieważ rzutując, rzutujemy literały, które i tak są z definicji const (napisanie var as to nadal rzutowanie wartości zmiennej - literału - a nie samej zmiennej) *)
postfix     = primary , {
			  ( "as" , type )
		    | ( "(" , [ arg_list ] , ")" )	
			| ( "[" , expression , "]" )
			| "!"
			} ;
			
primary     = identifier
			| int_lit
			| flp_lit
			| str_lit
			| bool_lit
			| arr_lit
			| ( "(" , expression , ")" ) ;

arg_list    = expression , { ",", expression } ;
```

### Instrukcje, zakresy, funkcje

```EBNF
scope       = "{" , [ newline ] , { statement } , "}" ;
scope_in_func   = "{" , [ newline ] , { stmt_in_func }, "}"

if_stmt     = "if" , "(" , expression , ")" , [ newline ] ,
			  ( scope_in_func | simple_stmt ) , 
			  [ [ newline ] , "else" , [ newline ] , 
			    ( scope_in_func | simple_stmt ) ] ;
			  
while_stmt  = "while" , "(" , expression , ")" , [ newline ] ,
			  ( scope_in_func | simple_stmt ) ;

func_params = type , identifier , { "," , type , identifier } ;
func_decl   = ( type | "void" ) , identifier , 
				"(" , [ func_params ] , ")" , [ newline ] , 
				scope_in_func ;

ret_stmt    = "return" , [ expression ] ;

simple_stmt     = var_decl
				| expression
				| ret_stmt 

(* Zakresy i instrukcje bez deklaracji funkcji *)
(* Wprowadziłem ten podział, aby zakaz deklaracji funkcji w funkcjach *)
stmt_in_func    = simple_stmt , newline
				| if_stmt, newline
				| while_stmt, newline
				| scope_in_func , newline

statement       = var_decl , newline
				| expression , newline 
				| ret_stmt , newline
				| if_stmt , newline
				| while_stmt , newline
				| func_decl , newline
				| scope , newline ;	

program         = { statement } ;
```