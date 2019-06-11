#include <stdio.h>

struct main_menu_options{
	int option_id;
	void *function_to_execute;
	char *menu_string;
};
/*TODO: Replace function_n with the actual functions I want to call*/
struct main_menu_options main_menu_options_array[4] = {
	{0x0, function_0, "[0] Turn on/off warning lights\n"},
	{0x1, function_1, "[1] Change RPM value\n"},
	{0x2, function_2, "[2] Change the speed\n"},
	{0x3, function_3, "[3] Change the gear\n"}
};

int main(int argc, char **argv){
	while(1/*program should run forever*/ ){
		;
		/*print options for user to choose from*/	
		/*process user options*/
	}
}

static int print_main_menu(void){
	printf( "What would you like to do?\n"
		"[0] Turn on/off warning lights\n"
		"[1] Change RPM value\n"
		"[2] Change the speed\n"
		"[3] Change the gear\n");
}


