#include <stdio.h>
#include <string.h>
#include <stdint.h>

//if message is a uint8_t array, it should be passed as ((char *) message)
void Append_Float_To_String(char *message, float data) {
    char temp[32];
    sprintf(temp, "%f", data);
    snprintf(temp, sizeof(temp), "%f", data);
    size_t used = strlen(message);
    size_t remaining = sizeof(message) - used;
    strncat(message, temp, remaining - 1);
}

// int main(void) {
//     float acc_x = 33.33;
//     char message[256] = "The acc_x data is ";
//     // uint8_t new_message[strlen((char *) message) + 1U];

//     // uint8_t temp[32];
//     // sprintf(temp, "%f", acc_x);

//     // strcat(message, temp);

//     Append_Float_To_String(message, acc_x);
//     char message_new[] = "The acc_x data is ";

//     printf("%s\n", message);
//     printf("%zi\n", sizeof(message_new));
// }

// int main(void) {
//     float var_1 = 0.03;
//     uint32_t var_2 = 3;
//     if ((var_1 + var_2) > 3) {
//         printf("passed %f", (var_1 + var_2));
//     }
// }

