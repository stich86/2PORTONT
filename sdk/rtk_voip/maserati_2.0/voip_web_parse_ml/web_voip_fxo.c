#include <stdio.h>
#include "web_voip.h"

char cid_det[CID_MAX][25] = {"FSK_BELLCORE", "FSK_ETSI", "FSK_BT", "FSK_NTT", "DTMF"};

extern int voip_char_replace(const char *src, char old_char, char new_char, char *result);

void asp_fxo_volumne(request * wp, int nVolumne)
{
	int i;

	for(i=0; i<10; i++)
	{
		if (i == nVolumne)
			boaWrite(wp, "<option selected>%d</option>", (i+1));
		else
			boaWrite(wp, "<option>%d</option>", (i+1));
	}
}


void asp_voip_FxoSet(request * wp, char *path, char *query)
{
	voipCfgParam_t *pCfg;
	char key_display[FUNC_KEY_LENGTH];
	char *ptr;
	int i;

	if (web_flash_get(&pCfg) != 0)
		return;

	/* PSTN Function Key */
	strcpy(key_display, boaGetVar(wp, "funckey_pstn", "*0"));
	voip_char_replace(key_display, '*', '.', pCfg->funckey_pstn);

	/* FXO valume */
	pCfg->daa_txVolumne = atoi(boaGetVar(wp, "daa_txVolumne", "1"));
	pCfg->daa_rxVolumne = atoi(boaGetVar(wp, "daa_rxVolumne", "1"));

	/* FXO Caller ID Detection */
	pCfg ->cid_auto_det_select = atoi( boaGetVar(wp, "caller_id_auto_det", "") );

	ptr	 = boaGetVar(wp, "caller_id_det", "");
	for(i=0; i<CID_MAX; i++)
	{
		if (!gstrcmp(ptr, cid_det[i]))
			break;
	}
	if (i == CID_MAX)
		i = CID_DTMF;

	pCfg->caller_id_det_mode = i;


	web_flash_set(pCfg);
#ifdef REBOOT_CHECK
	OK_MSG("/voip_fxo.asp");
#else
	web_restart_solar();
	boaRedirect(wp, "/voip_fxo.asp");
#endif


}

#ifdef CONFIG_APP_BOA
int asp_voip_FxoGet(request * wp, int argc, char **argv)
#else
int asp_voip_FxoGet(int ejid, request * wp, int argc, char **argv)
#endif
{
	voipCfgParam_t *pCfg;
	char key_display[FUNC_KEY_LENGTH];
	int i;

	if (web_flash_get(&pCfg) != 0)
		return -1;

	if (strcmp(argv[0], "funckey_pstn")==0)
	{
		voip_char_replace(pCfg->funckey_pstn, '.', '*', key_display);
		boaWrite(wp, "%s", key_display);
	}
	else if (strcmp(argv[0], "daa_txVolumne")==0)
		asp_fxo_volumne(wp, pCfg->daa_txVolumne - 1);
	else if (strcmp(argv[0], "daa_rxVolumne")==0)
		asp_fxo_volumne(wp, pCfg->daa_rxVolumne - 1);
	else if (strcmp(argv[0], "caller_id_auto_det")==0)
	{
		boaWrite(wp, "<input type=\"radio\" name=\"caller_id_auto_det\" value=0 onClick=enable_cid_det_mode() %s>Off",
    				( pCfg ->cid_auto_det_select == 0 ? "checked" : "" ) );
		boaWrite(wp, "<input type=\"radio\" name=\"caller_id_auto_det\" value=1 onClick=enable_cid_det_mode() %s>On (NTT Support)",
    				( pCfg ->cid_auto_det_select == 1 ? "checked" : "" ) );
    		boaWrite(wp, "<input type=\"radio\" name=\"caller_id_auto_det\" value=2 onClick=enable_cid_det_mode() %s>On (NTT Not Support)",
    				( pCfg ->cid_auto_det_select == 2 ? "checked" : "" ) );
	}
	else if (strcmp(argv[0], "caller_id_det")==0)
	{
		for (i=0; i<CID_MAX ;i++)
		{
			if (i == (pCfg->caller_id_det_mode))
				boaWrite(wp, "<option selected>%s</option>", cid_det[i]);
			else
				boaWrite(wp, "<option>%s</option>", cid_det[i]);

		}

	}
	else if (strcmp(argv[0], "display_funckey_pstn")==0)
	{
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
		boaWrite(wp, "%s", "");
#else
		boaWrite(wp, "%s", "style=\"display:none\"");
#endif
	}
#if defined(CONFIG_RTK_VOIP_DRIVERS_FXO) && !defined(CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA)
	else if (strcmp(argv[0], "display_fxo")==0)
		boaWrite(wp, "%s", "");
#else
	else if (strcmp(argv[0], "display_fxo")==0)
		boaWrite(wp, "%s", "style=\"display:none\"");
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
	else if (strcmp(argv[0], "display_cid_det")==0)
		boaWrite(wp, "%s", "");
#else
	else if (strcmp(argv[0], "display_cid_det")==0)
		boaWrite(wp, "%s", "style=\"display:none\"");
#endif
	else
	{
		return -1;
	}

	return 0;

}

