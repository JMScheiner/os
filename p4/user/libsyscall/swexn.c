/* This file exists ONLY so user programs link without errors.
 * Do NOT turn it in (figure out something sensible to do
 * instead).
 */

#include <syscall.h>

int swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
{
  return (-1);
}
