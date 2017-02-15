/******************************************************************************
*
* @brief provide low level start up routine. 
*
*******************************************************************************/

;         AREA   Crt0, CODE, READONLY      ; name this block of code
  SECTION .noinit : CODE

        EXPORT  __startup
__startup
        B       __startup

        END
