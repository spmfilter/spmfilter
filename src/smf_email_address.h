/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2010 Axel Steiner and SpaceNet AG
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SMF_EMAIL_ADDRESS_H
#define _SMF_EMAIL_ADDRESS_H

#include <glib.h>

#include "spmfilter_config.h"
#include "smf_lookup.h"

typedef struct {
    char *addr;
    int is_local;
    SMFLookupResult_T *lr;
} SMFEmailAddress_T;

/*!
 * @fn SMFEmailAddress_T *smf_email_address_new(void)
 * @brief Creates a new SMFEmailAddress_T object
 * @returns an empty SMFEmailAddress_T object
 */
SMFEmailAddress_T *smf_email_address_new(void);

/*!
 * @fn void smf_email_address_free(SMFEmailAddress_T *ea)
 * @brief Free a SMFEmailAddress_T object
 * @param ea a SMFEmailAddress_T object
 */
void smf_email_address_free(SMFEmailAddress_T *ea);

/*!
 * @fn SMFEmailAddress_T *smf_email_address_set_addr(SMFEmailAddress_T *ea, char *addr)
 * @brief Set the address of a SMFEmailAddress_T object
 * @param ea SMFEmailAddress_T object
 * @param addr E-Mail address
 * @returns SMFEmailAddress_T object
 */
SMFEmailAddress_T *smf_email_address_set_addr(SMFEmailAddress_T *ea, char *addr);

/*!
 * @fn char *smf_email_address_get_addr(SMFEmailAddress_T *ea)
 * @brief Get E-Mail address of SMFEmailAddress_T object
 * @param ea SMFEmailAddress_T object
 * @returns E-Mail Address
 */
char *smf_email_address_get_addr(SMFEmailAddress_T *ea);

/*!
 * @fn SMFEmailAddress_T *smf_email_address_set_lr(SMFEmailAddress_T *ea, SMFLookupResult_T *lr)
 * @brief Set the lookup result for a SMFEmailAddress_T obejct
 * @param ea SMFEmailAddress_T object
 * @param lr SMFLookupResult_T object
 * @returns SMFEmailAddress_T object
 */
SMFEmailAddress_T *smf_email_address_set_lr(SMFEmailAddress_T *ea, SMFLookupResult_T *lr);

/*!
 * @fn SMFLookupResult_T *smf_email_address_get_lr(SMFEmailAddress_T *ea)
 * @brief Get a SMFLookupResult_T object of a SMFEmailAddress_T object
 * @param ea SMFEMailAddress_T obejct 
 * @returns SMFLookupResult_T object
 */
SMFLookupResult_T *smf_email_address_get_lr(SMFEmailAddress_T *ea);

#endif  /* _SMF_EMAIL_ADDRESS_H */
