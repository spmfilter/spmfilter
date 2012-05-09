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

#ifndef _SMF_SETTINGS_PRIVATE_H
#define	_SMF_SETTINGS_PRIVATE_H

#include "smf_settings.h"

SMFSettings_T *smf_settings_new(void);

/** free settings struct */
void smf_settings_free(SMFSettings_T *settings);

/** load and parse config file
 *
 * \param settings SMFSettings_T object
 * \param alternate_file path to alternate config file
 *
 * \returns 0 on success or -1 in case of error
 */

int smf_settings_parse_config(SMFSettings_T **settings, char *alternate_file);

#endif	/* _SMF_SETTINGS_PRIVATE_H */
