/*! \file bcmpc_imm_port.c
 *
 * PC_PORTt IMM handler.
 */
/*
 * Copyright: (c) 2018 Broadcom. All Rights Reserved. "Broadcom" refers to 
 * Broadcom Limited and/or its subsidiaries.
 * 
 * Broadcom Switch Software License
 * 
 * This license governs the use of the accompanying Broadcom software. Your 
 * use of the software indicates your acceptance of the terms and conditions 
 * of this license. If you do not agree to the terms and conditions of this 
 * license, do not use the software.
 * 1. Definitions
 *    "Licensor" means any person or entity that distributes its Work.
 *    "Software" means the original work of authorship made available under 
 *    this license.
 *    "Work" means the Software and any additions to or derivative works of 
 *    the Software that are made available under this license.
 *    The terms "reproduce," "reproduction," "derivative works," and 
 *    "distribution" have the meaning as provided under U.S. copyright law.
 *    Works, including the Software, are "made available" under this license 
 *    by including in or with the Work either (a) a copyright notice 
 *    referencing the applicability of this license to the Work, or (b) a copy 
 *    of this license.
 * 2. Grant of Copyright License
 *    Subject to the terms and conditions of this license, each Licensor 
 *    grants to you a perpetual, worldwide, non-exclusive, and royalty-free 
 *    copyright license to reproduce, prepare derivative works of, publicly 
 *    display, publicly perform, sublicense and distribute its Work and any 
 *    resulting derivative works in any form.
 * 3. Grant of Patent License
 *    Subject to the terms and conditions of this license, each Licensor 
 *    grants to you a perpetual, worldwide, non-exclusive, and royalty-free 
 *    patent license to make, have made, use, offer to sell, sell, import, and 
 *    otherwise transfer its Work, in whole or in part. This patent license 
 *    applies only to the patent claims licensable by Licensor that would be 
 *    infringed by Licensor's Work (or portion thereof) individually and 
 *    excluding any combinations with any other materials or technology.
 *    If you institute patent litigation against any Licensor (including a 
 *    cross-claim or counterclaim in a lawsuit) to enforce any patents that 
 *    you allege are infringed by any Work, then your patent license from such 
 *    Licensor to the Work shall terminate as of the date such litigation is 
 *    filed.
 * 4. Redistribution
 *    You may reproduce or distribute the Work only if (a) you do so under 
 *    this License, (b) you include a complete copy of this License with your 
 *    distribution, and (c) you retain without modification any copyright, 
 *    patent, trademark, or attribution notices that are present in the Work.
 * 5. Derivative Works
 *    You may specify that additional or different terms apply to the use, 
 *    reproduction, and distribution of your derivative works of the Work 
 *    ("Your Terms") only if (a) Your Terms provide that the limitations of 
 *    Section 7 apply to your derivative works, and (b) you identify the 
 *    specific derivative works that are subject to Your Terms. 
 *    Notwithstanding Your Terms, this license (including the redistribution 
 *    requirements in Section 4) will continue to apply to the Work itself.
 * 6. Trademarks
 *    This license does not grant any rights to use any Licensor's or its 
 *    affiliates' names, logos, or trademarks, except as necessary to 
 *    reproduce the notices described in this license.
 * 7. Limitations
 *    Platform. The Work and any derivative works thereof may only be used, or 
 *    intended for use, with a Broadcom switch integrated circuit.
 *    No Reverse Engineering. You will not use the Work to disassemble, 
 *    reverse engineer, decompile, or attempt to ascertain the underlying 
 *    technology of a Broadcom switch integrated circuit.
 * 8. Termination
 *    If you violate any term of this license, then your rights under this 
 *    license (including the license grants of Sections 2 and 3) will 
 *    terminate immediately.
 * 9. Disclaimer of Warranty
 *    THE WORK IS PROVIDED "AS IS" WITHOUT WARRANTIES OR CONDITIONS OF ANY 
 *    KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WARRANTIES OR CONDITIONS OF 
 *    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE OR 
 *    NON-INFRINGEMENT. YOU BEAR THE RISK OF UNDERTAKING ANY ACTIVITIES UNDER 
 *    THIS LICENSE. SOME STATES' CONSUMER LAWS DO NOT ALLOW EXCLUSION OF AN 
 *    IMPLIED WARRANTY, SO THIS DISCLAIMER MAY NOT APPLY TO YOU.
 * 10. Limitation of Liability
 *    EXCEPT AS PROHIBITED BY APPLICABLE LAW, IN NO EVENT AND UNDER NO LEGAL 
 *    THEORY, WHETHER IN TORT (INCLUDING NEGLIGENCE), CONTRACT, OR OTHERWISE 
 *    SHALL ANY LICENSOR BE LIABLE TO YOU FOR DAMAGES, INCLUDING ANY DIRECT, 
 *    INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF 
 *    OR RELATED TO THIS LICENSE, THE USE OR INABILITY TO USE THE WORK 
 *    (INCLUDING BUT NOT LIMITED TO LOSS OF GOODWILL, BUSINESS INTERRUPTION, 
 *    LOST PROFITS OR DATA, COMPUTER FAILURE OR MALFUNCTION, OR ANY OTHER 
 *    COMMERCIAL DAMAGES OR LOSSES), EVEN IF THE LICENSOR HAS BEEN ADVISED OF 
 *    THE POSSIBILITY OF SUCH DAMAGES.
 */


#include <shr/shr_error.h>
#include <shr/shr_debug.h>
#include <bcmltd/chip/bcmltd_id.h>
#include <bcmimm/bcmimm_int_comp.h>

#include <bcmpc/bcmpc_util_internal.h>
#include <bcmpc/bcmpc_imm_internal.h>
#include <bcmpc/bcmpc_pm_internal.h>
#include <bcmpc/bcmpc_lport_internal.h>
#include <bcmpc/bcmpc_pm.h>


/*******************************************************************************
 * Local definitions
 */

/* Debug log target definition */
#define BSL_LOG_MODULE BSL_LS_BCMPC_IMM

/*! Logical table ID for this handler. */
#define IMM_SID PC_PORTt

/*! The data structure for PC_PORTt entry. */
typedef struct pc_port_entry_s {
    bcmpc_lport_t key;
    bcmpc_port_cfg_t data;
} pc_port_entry_t;


/*******************************************************************************
 * Private Functions
 */

 /*!
  * \brief Initialize pc_port_entry_t data buffer.
  *
  * \param [in] entry entry buffer.
  *
  * \return Nothing.
  */
static void
pc_port_entry_t_init(pc_port_entry_t *entry)
{
    entry->key = BCMPC_INVALID_LPORT;
    bcmpc_port_cfg_t_init(&entry->data);
}

/*!
 * \brief Fill PC structure according to the given LT field value.
 *
 * \param [in] unit Unit number.
 * \param [in] fid Field ID.
 * \param [in] fval Field value.
 * \param [in,out] entry Entry buffer.
 *
 * \return SHR_E_NONE on success, error code otherwise.
 */
static int
pc_port_entry_set(int unit, uint32_t fid, uint64_t fval,
                  pc_port_entry_t *entry)
{
    bcmpc_port_cfg_t *pcfg = &entry->data;

    SHR_FUNC_ENTER(unit);

    switch (fid) {
        case PC_PORTt_PORT_IDf:
            entry->key = fval;
            break;
        case PC_PORTt_PC_PHYS_PORT_IDf:
            pcfg->pport = fval;
            break;
        case PC_PORTt_ENABLEf:
            pcfg->enable = fval;
            break;
        case PC_PORTt_OPMODEf:
            pcfg->opmode = fval;
            break;
        case PC_PORTt_PC_AUTONEG_PROFILE_IDf:
            pcfg->autoneg_profile_id = fval;
            break;
        case PC_PORTt_PC_CUSTOM_OPMODE_IDf:
            pcfg->custom_opmode_id = fval;
            break;
        case PC_PORTt_ENCAPf:
            pcfg->encap_mode = fval;
            break;
        case PC_PORTt_LOOPBACK_MODEf:
            pcfg->lpbk_mode = fval;
            break;
        case PC_PORTt_PAUSE_ADDRf:
            sal_memcpy(pcfg->pause_addr, &fval, sizeof(pcfg->pause_addr));
            break;
        case PC_PORTt_PAUSE_TXf:
            pcfg->pause_tx = fval;
            break;
        case PC_PORTt_PAUSE_RXf:
            pcfg->pause_rx = fval;
            break;
        case PC_PORTt_MAX_FRAME_SIZEf:
            pcfg->max_frame_size = fval;
            break;
        case PC_PORTt_PC_PFC_PROFILE_IDf:
            pcfg->pfc_profile_id = fval;
            break;
        default:
            SHR_RETURN_VAL_EXIT(SHR_E_FAIL);
    }

exit:
    SHR_FUNC_EXIT();
}

/*!
 * \brief IMM input LT fields parsing.
 *
 * Parse the input LT fields and save the data to pc_port_entry_t.
 *
 * \param [in] unit Unit number.
 * \param [in] key_flds IMM input key field array.
 * \param [in] data_flds IMM input data field array.
 * \param [out] entry Port config data buffer.
 *
 * \retval SHR_E_NONE No errors.
 * \retval SHR_E_FAIL Faild to convert \c in to \c pdata.
 */
static int
pc_lt_port_fields_parse(int unit,
                        const bcmltd_field_t *key_flds,
                        const bcmltd_field_t *data_flds,
                        pc_port_entry_t *entry)
{
    const bcmltd_field_t *fld;
    uint32_t fid;
    uint64_t fval;

    SHR_FUNC_ENTER(unit);

    /* Parse key fields. */
    fld = key_flds;
    while (fld) {
        fid = fld->id;
        fval = fld->data;

        SHR_IF_ERR_VERBOSE_EXIT
            (pc_port_entry_set(unit, fid, fval, entry));

        fld = fld->next;
    }

    /* Parse data fields. */
    fld = data_flds;
    while (fld) {
        fid = fld->id;
        fval = fld->data;

        SHR_IF_ERR_VERBOSE_EXIT
            (pc_port_entry_set(unit, fid, fval, entry));

        fld = fld->next;
    }

exit:
    SHR_FUNC_EXIT();
}

/*!
 * \brief Do the internal hardware configuration for INSERT operation.
 *
 * \param [in] unit Unit number.
 * \param [out] entry Port config data buffer.
 *
 * \retval SHR_E_NONE No errors.
 * \retval !SHR_E_NONE Failure.
 */
static int
pc_lt_port_insert(int unit, pc_port_entry_t *entry)
{
    bcmpc_lport_t lport = entry->key;
    bcmpc_port_cfg_t *pcfg = &entry->data;

    SHR_FUNC_ENTER(unit);

    if (bcmpc_p2l_lport_get(unit, pcfg->pport) != BCMPC_INVALID_LPORT) {
        SHR_IF_ERR_VERBOSE_MSG_EXIT
            (SHR_E_FAIL,
             (BSL_META_UP(unit, lport,
                          "Physical port %d is already used by the other "
                          "logical port\n"),
              pcfg->pport));
    }

    SHR_IF_ERR_VERBOSE_EXIT
        (bcmpc_internal_port_insert(unit, lport, pcfg));

exit:
    SHR_FUNC_EXIT();
}

/*!
 * \brief Do the internal hardware configuration for UPDATE operation.
 *
 * \param [in] unit Unit number.
 * \param [out] entry Port config data buffer.
 *
 * \retval SHR_E_NONE No errors.
 * \retval !SHR_E_NONE Failure.
 */
static int
pc_lt_port_update(int unit, pc_port_entry_t *entry)
{
    int pm_id;
    bcmpc_pm_info_t pm_info;
    bcmpc_port_cfg_t cur_pcfg;
    bcmpc_lport_t lport = entry->key;
    bcmpc_port_cfg_t *pcfg = &entry->data;
    bcmpc_pport_t pport = pcfg->pport;

    SHR_FUNC_ENTER(unit);

    SHR_IF_ERR_VERBOSE_EXIT
        (bcmpc_topo_id_get(unit, pport, &pm_id));
    SHR_IF_ERR_VERBOSE_EXIT
        (bcmpc_pm_info_get(unit, pm_id, &pm_info));

    SHR_IF_ERR_VERBOSE_EXIT
        (bcmpc_db_imm_entry_lookup(unit, IMM_SID, P(&lport), P(&cur_pcfg)));

    if (pport != cur_pcfg.pport) {
        SHR_IF_ERR_VERBOSE_MSG_EXIT
            (SHR_E_FAIL,
             (BSL_META_U(unit, "PC_PHYS_PORT_ID should not be changed.\n")));
    }

    SHR_IF_ERR_VERBOSE_EXIT
        (bcmpc_internal_port_set(unit, lport, &entry->data));

exit:
    SHR_FUNC_EXIT();
}

/*!
 * \brief Do the internal hardware configuration for DELETE operation.
 *
 * \param [in] unit Unit number.
 * \param [out] entry Port config data buffer.
 *
 * \retval SHR_E_NONE No errors.
 * \retval !SHR_E_NONE Failure.
 */
static int
pc_lt_port_delete(int unit, pc_port_entry_t *entry)
{
    SHR_FUNC_ENTER(unit);

    SHR_IF_ERR_VERBOSE_EXIT
        (bcmpc_internal_port_delete(unit, entry->key));

exit:
    SHR_FUNC_EXIT();
}


/*******************************************************************************
 * DB schema handler
 */

/*!
 * \brief Convert the LT fields to the PC data structure.
 *
 * This is the parse callback function of IMM DB schema handler, \ref
 * bcmpc_db_imm_schema_handler_t.
 *
 * Convert bcmltd_fields_t array \c flds to a bcmpc_lport_t \c key and a
 * bcmpc_port_cfg_t \c data.
 *
 * \param [in] unit Unit number.
 * \param [in] flds LT field array.
 * \param [in] key_size Key size.
 * \param [out] key Key value from \c flds.
 * \param [in] data_size Data size.
 * \param [out] data Data value from \c flds.
 *
 * \return SHR_E_NONE on success, error code otherwise.
 */
static int
pc_imm_port_entry_parse(int unit, const bcmltd_fields_t *flds,
                        size_t key_size, void *key,
                        size_t data_size, void *data)
{
    size_t i;
    uint32_t fid;
    uint64_t fval;
    pc_port_entry_t entry;

    SHR_FUNC_ENTER(unit);

    pc_port_entry_t_init(&entry);

    for (i = 0; i < flds->count; i++) {
        fid = flds->field[i]->id;
        fval = flds->field[i]->data;
        SHR_IF_ERR_VERBOSE_EXIT
            (pc_port_entry_set(unit, fid, fval, &entry));
    }

    if (key) {
        sal_memcpy(key, &entry.key, key_size);
    }

    if (data) {
        sal_memcpy(data, &entry.data, data_size);
    }

exit:
    SHR_FUNC_EXIT();
}

/*!
 * \brief Convert the PC data structure to the LT fields.
 *
 * This is the fill callback function of IMM DB schema handler, \ref
 * bcmpc_db_imm_schema_handler_t.
 *
 * Convert a bcmpc_lport_t \c key and a bcmpc_port_cfg_t \c data to
 * bcmltd_fields_t array \c flds.
 *
 * \param [in] unit Unit number.
 * \param [in] key_size Key size.
 * \param [in] key Key value.
 * \param [in] data_size Data size.
 * \param [in] data Data value.
 * \param [out] flds LT field array.
 *
 * \return SHR_E_NONE on success, error code otherwise.
 */
static int
pc_imm_port_entry_fill(int unit,
                       size_t key_size, void *key,
                       size_t data_size, void *data,
                       bcmltd_fields_t *flds)
{
    size_t fcnt = 0;
    bcmpc_lport_t *lport = key;
    bcmpc_port_cfg_t *pcfg = data;

    SHR_FUNC_ENTER(unit);

    if (key) {
        BCMPC_LT_FIELD_VAL_ADD
            (flds, PC_PORTt_PORT_IDf, 0, *lport, fcnt);
    }

    if (data) {
        BCMPC_LT_FIELD_VAL_ADD
            (flds, PC_PORTt_PC_PHYS_PORT_IDf, 0, pcfg->pport, fcnt);
        BCMPC_LT_FIELD_VAL_ADD
            (flds, PC_PORTt_ENABLEf, 0, pcfg->enable, fcnt);
        BCMPC_LT_FIELD_VAL_ADD
            (flds, PC_PORTt_OPMODEf, 0, pcfg->opmode, fcnt);
        BCMPC_LT_FIELD_VAL_ADD
            (flds, PC_PORTt_PC_AUTONEG_PROFILE_IDf, 0, pcfg->autoneg_profile_id,
             fcnt);
        BCMPC_LT_FIELD_VAL_ADD
            (flds, PC_PORTt_PC_CUSTOM_OPMODE_IDf, 0, pcfg->custom_opmode_id,
             fcnt);
        BCMPC_LT_FIELD_VAL_ADD
            (flds, PC_PORTt_PC_PFC_PROFILE_IDf, 0, pcfg->pfc_profile_id, fcnt);
        BCMPC_LT_FIELD_VAL_ADD
            (flds, PC_PORTt_ENCAPf, 0, pcfg->encap_mode, fcnt);
        BCMPC_LT_FIELD_VAL_ADD
            (flds, PC_PORTt_LOOPBACK_MODEf, 0, pcfg->lpbk_mode, fcnt);
        BCMPC_LT_FIELD_DATA_ADD
            (flds, PC_PORTt_PAUSE_ADDRf, 0, &pcfg->pause_addr, fcnt);
        BCMPC_LT_FIELD_VAL_ADD
            (flds, PC_PORTt_PAUSE_TXf, 0, pcfg->pause_tx, fcnt);
        BCMPC_LT_FIELD_VAL_ADD
            (flds, PC_PORTt_PAUSE_RXf, 0, pcfg->pause_rx, fcnt);
        BCMPC_LT_FIELD_VAL_ADD
            (flds, PC_PORTt_MAX_FRAME_SIZEf, 0, pcfg->max_frame_size, fcnt);
    }

    flds->count = fcnt;

    SHR_FUNC_EXIT();
}


/*******************************************************************************
 * IMM event handler
 */

/*!
 * \brief Stage callback function of IMM event handler (bcmimm_lt_cb_t).
 *
 * \param [in] unit This is device unit number.
 * \param [in] sid This is the logical table ID.
 * \param [in] trans_id is the transaction ID associated with this operation.
 * \param [in] event This is the reason for the entry event.
 * \param [in] key_flds This is a linked list of the key fields identifying
 *                        the entry.
 * \param [in] data_flds This is a linked list of the data fields in the
 *                        modified entry.
 * \param [in] context Is a pointer that was given during registration.
 *                     The callback can use this pointer to retrieve some
 *                     context.
 *
 * \return SHR_E_NONE on success and error code otherwise.
 */
static int
pc_port_stage(int unit,
              bcmltd_sid_t sid,
              uint32_t trans_id,
              bcmimm_entry_event_t event,
              const bcmltd_field_t *key_flds,
              const bcmltd_field_t *data_flds,
                 void *context,
                 bcmltd_fields_t *output_fields)
{
    pc_port_entry_t entry;

    SHR_FUNC_ENTER(unit);

    pc_port_entry_t_init(&entry);
    SHR_IF_ERR_VERBOSE_EXIT
        (pc_lt_port_fields_parse(unit, key_flds, NULL, &entry));

    if (output_fields) {
        output_fields->count = 0;
    }
    switch (event) {
        case BCMIMM_ENTRY_INSERT:
            SHR_IF_ERR_VERBOSE_EXIT
                (pc_lt_port_fields_parse(unit, NULL, data_flds, &entry));
            SHR_IF_ERR_VERBOSE_EXIT
                (pc_lt_port_insert(unit, &entry));
            break;
        case BCMIMM_ENTRY_UPDATE:
            SHR_IF_ERR_VERBOSE_EXIT
                (bcmpc_db_imm_entry_lookup(unit,
                                           IMM_SID,
                                           P(&entry.key), P(&entry.data)));
            SHR_IF_ERR_VERBOSE_EXIT
                (pc_lt_port_fields_parse(unit, NULL, data_flds, &entry));
            SHR_IF_ERR_VERBOSE_EXIT
                (pc_lt_port_update(unit, &entry));
            break;
        case BCMIMM_ENTRY_DELETE:
            SHR_IF_ERR_VERBOSE_EXIT
                (pc_lt_port_delete(unit, &entry));
            break;
        default:
            SHR_RETURN_VAL_EXIT(SHR_E_PARAM);
    }

exit:
    SHR_FUNC_EXIT();
}


/*******************************************************************************
 * Public Functions
 */

int
bcmpc_imm_port_register(int unit)
{
    bcmpc_db_imm_schema_handler_t db_hdl;
    bcmimm_lt_cb_t event_hdl;

    SHR_FUNC_ENTER(unit);

    db_hdl.entry_parse = pc_imm_port_entry_parse;
    db_hdl.entry_fill = pc_imm_port_entry_fill;

    event_hdl.validate = NULL;
    event_hdl.stage = pc_port_stage;
    event_hdl.commit = NULL;
    event_hdl.abort = NULL;

    /* Register the handlers to the table. */
    SHR_IF_ERR_EXIT
        (bcmpc_imm_tbl_register(unit, IMM_SID, &db_hdl, &event_hdl));

exit:
    SHR_FUNC_EXIT();
}

