#include "../Initializes/initialize.h" // IWYU pragma: keep


/*              STATE                    TYPE            VALUE           ADDR        
            1. Program state            uint8_t         0-4 (enum)      0x0000      
            2. Dispenser position       uint8_t         0-7 (int)       0x0002      
            3. Average steps            uint16_t        ~4096 (int)     0x0004      
            4  Dispenser is moving      uint8_t         0-1 (bool)      0x0008      
*/




// Program state is written everytime it's changed


// Dispenser position is written everytime it finishes moving a 1/8 step 


// Average steps takes 2 bytes of memory and is written:    
//      after calibration: avg_steps 
//      when dispensing is done wite 0 to indicate that calibration haven't been done


// Dispenser is moving is written everytime dispenser starts moving and when it stops

