/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <runtime_svc.h>
#include <debug.h>
#include <oem_svc.h>
#include <platform.h>
#include <stdint.h>
#include <uuid.h>

/* OEM Service UUID */
DEFINE_SVC_UUID(oem_svc_uid,
		0xb943add0, 0x069d, 0x11e4, 0x91, 0x91,
		0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66);


/* Setup Standard Services */
static int32_t oem_svc_setup(void)
{
	/*
	 * Invoke related module setup from here
	 */

	return 0;
}

/*******************************************************************************
 * OEM top level handler for servicing SMCs.
 ******************************************************************************/
uint64_t oem_smc_handler(uint32_t smc_fid,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags)
{
	uint64_t rc;    
	uint32_t ns;
    
	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);

	switch (smc_fid) {
	case OEM_SMC_CALL_1_AARCH32:
	case OEM_SMC_CALL_1_AARCH64:		
        /* Don't allow the SMC call from non-secure world */
        if (ns)
			SMC_RET1(handle, SMC_UNK);
		rc = OEM_SVC_E_NOT_SUPPORTED;
		break;    
	case OEM_SMC_CALL_2_AARCH32:
	case OEM_SMC_CALL_2_AARCH64:
		rc = OEM_SVC_E_NOT_SUPPORTED;
		break;    
	case OEM_SMC_CALL_3_AARCH32:
	case OEM_SMC_CALL_3_AARCH64:
		rc = OEM_SVC_E_NOT_SUPPORTED;
		break;    
	default:
		rc = SMC_UNK;
		WARN("Unimplemented OEM Call: 0x%x \n", smc_fid);
	}

	SMC_RET1(handle, rc);
}

/*
 * Top-level Standard Service SMC handler. This handler will in turn dispatch
 * calls to related SMC handler
 */
uint64_t oem_svc_smc_handler(uint32_t smc_fid,
			     uint64_t x1,
			     uint64_t x2,
			     uint64_t x3,
			     uint64_t x4,
			     void *cookie,
			     void *handle,
			     uint64_t flags)
{
	/*
	 * Dispatch OEM calls to OEM Common handler and return its return value
	 */
	if (is_oem_fid(smc_fid)) {
		return oem_smc_handler(smc_fid, x1, x2, x3, x4, cookie,
				handle, flags);
	}

	switch (smc_fid) {
	case OEM_SVC_CALL_COUNT:
		/*
		 * Return the number of Standard Service Calls. PSCI is the only
		 * standard service implemented; so return number of PSCI calls
		 */
		SMC_RET1(handle, OEM_SVC_NUM_CALLS);

	case OEM_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, oem_svc_uid);

	case OEM_SVC_VERSION:
		/* Return the version of current implementation */
		SMC_RET2(handle, OEM_VERSION_MAJOR, OEM_VERSION_MINOR);

	default:
		WARN("Unimplemented OEM Service Call: 0x%x \n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

/* Register Standard Service Calls as runtime service */
DECLARE_RT_SVC(
		oem_svc,

		OEN_OEM_START,
		OEN_OEM_END,
		SMC_TYPE_FAST,
		oem_svc_setup,
		oem_svc_smc_handler
);
