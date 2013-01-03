/* spmfilter - mail filtering framework
 * Copyright (C) 2012 Robin Doer and SpaceNet AG
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
#ifndef _SMF_NEXTHOP_H
#define _SMF_NEXTHOP_H

#include "smf_settings.h"
#include "smf_session.h"

/**
 * Prototype of the nexthop-function.
 *
 * Invoking this function will forward the message in the given session to a
 * configurable destination (depends on the implementation of the
 * nexthop-function).
 */
typedef int (*NexthopFunction)(SMFSettings_T *settings, SMFSession_T *session);

/**
 * Depending on the configuration finds and returns the nexthop-action.
 *
 * Usually the nextjop will forward an email to a destination. If NULL is returned,
 * then no nexthop-action shoudl be performed.
 *
 * @param settings the settiogs. Containts the configuration of the nexthop-action.
 * @return The nexthop-action. Invoking this function will perform the operation.
 *         If NULL is returned, then no hexthop-action is configured.
 */
NexthopFunction smf_nexthop_find(SMFSettings_T *settings);

#endif  /* _SMF_NEXTHOP_H */
