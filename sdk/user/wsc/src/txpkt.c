/*
 *  Transmit packet module for WiFi Simple-Config
 *
 *	Copyright (C)2006, Realtek Semiconductor Corp. All rights reserved.
 *
 *	$Id: txpkt.c,v 1.17 2012/02/08 13:53:52 krammer Exp $
 */

/*================================================================*/
/* Include Files */

#include "wsc.h"


/*================================================================*/
/* Implementation Routines */

#ifdef DEBUG
static const char *Hex="0123456789ABCDEF";
static int bn_printxxxx(const BIGNUM *a)
{		
	int i,j,v,z=0;
	int ret=0;

	if (a->neg)
		printf("-");
		
	if (a->top == 0) 
		printf("0");
	
	for (i=a->top-1; i >=0; i--)
		{
		for (j=BN_BITS2-4; j >= 0; j-=4)
			{
			/* strip leading zeros */
			v=((int)(a->d[i]>>(long)j))&0x0f;
			if (z || (v != 0))
				{
				printf("%c", Hex[v]);				
				z=1;
				}
			}
		}
	ret=1;

	return(ret);
}
#endif // DEBUG

#ifdef WPS2DOTX
int add_v2andAuthTag(CTX_Tp pCtx )
{
	int ixx = 0 ;
	memcpy(pCtx->VENDOR_DTAT , WSC_VENDOR_V2 , 6 );	/*WFA VENDOR OUI + V2 tag*/

	ixx =  report_authoriedMacCount(pCtx);	

	pCtx->VENDOR_DTAT[6]=VENDOR_AUTHMAC;
	pCtx->VENDOR_DTAT[7]=ixx*6;	
	memcpy(&pCtx->VENDOR_DTAT[8] , (void *)pCtx->authorized_macs ,ixx*6 );
	return (6+2+ixx*6)	;
}
#endif


static void build_wsc_msg_header(CTX_Tp pCtx, STA_CTX_Tp pSta, int op_code,
		struct eapol_t **o_eapol, struct eap_t **o_eap, struct eap_wsc_t **o_wsc)
{
	struct eapol_t *eapol;
	struct eap_t *eap;
	struct ethernet_t *eth_hdr;
	struct eap_wsc_t *wsc;

	memset(pSta->tx_buffer, '\0', TX_BUFFER_SIZE);
	eth_hdr = (struct ethernet_t *)pSta->tx_buffer;
	memcpy(eth_hdr->ether_dhost, pSta->addr, ETHER_ADDRLEN);

#ifdef FOR_DUAL_BAND
	if(pCtx->InterFaceComeIn == COME_FROM_WLAN1){
		memcpy(eth_hdr->ether_shost, pCtx->our_addr2, ETHER_ADDRLEN);
	}
	else if(pCtx->InterFaceComeIn == COME_FROM_WLAN0){
		memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);
	}
	else {
		memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);		
	}
#else //FOR_DUAL_BAND
	memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);
#endif //FOR_DUAL_BAND

	eth_hdr->ether_type = htons(ETHER_EAPOL_TYPE);

	eapol = (struct eapol_t *)(pSta->tx_buffer + ETHER_HDRLEN);
	eapol->protocol_version = EAPOL_VER;
	eapol->packet_type = EAPOL_EAPPKT;

	eap = (struct eap_t *)(((unsigned char *)eapol) + EAPOL_HDRLEN);
	if (pCtx->is_ap)
		eap->code =  EAP_REQUEST;
	else
		eap->code =  EAP_RESPONSE;
	
	if (pCtx->is_ap){
		pSta->eap_reqid+=1;
		DEBUG_PRINT2("AP's identifier ++ to:0x%x\n",pSta->eap_reqid);		
	}
	eap->identifier = htons(pSta->eap_reqid);

	wsc = (struct eap_wsc_t *)(((unsigned char *)eap) + EAP_HDRLEN);
	wsc->type =  EAP_TYPE_EXPANDED;
	memcpy(wsc->vendor_id, WSC_VENDOR_ID, 3);
	wsc->vendor_type = htonl(WSC_VENDOR_TYPE);
	wsc->op_code = op_code;

	*o_eapol = eapol;
	*o_eap = eap;
	*o_wsc = wsc;
}

static int add_encrypt_attr(STA_CTX_Tp pSta, unsigned char *data, int len, unsigned char *out)
{
	int size;
	unsigned char tmp[100];

	hmac_sha256(data, len, pSta->auth_key, BYTE_LEN_256B, tmp, &size);
	add_tlv(&data[len], TAG_KEY_WRAP_AUTH, BYTE_LEN_64B, (void *)tmp);

	generate_random(out, BYTE_LEN_128B); // iv

#ifdef DEBUG
	CTX_Tp pCtx = pGlobalCtx;
	if(pCtx->debug2) {
		debug_out("generate_random IV", out, BYTE_LEN_128B);
		debug_out("encrypted setting", data, len+BYTE_LEN_64B+4);
	}
#endif	
	Encrypt_aes_128_cbc(pSta->key_wrap_key, out, data, len+BYTE_LEN_64B+4, 
		&out[BYTE_LEN_128B], &size);
		
	return (size+BYTE_LEN_128B);
}

static int add_credential_attr(CTX_Tp pCtx, STA_CTX_Tp pSta, char *out, int include_cred)
{
	unsigned char byteVal, *pMsg;
	unsigned short shortVal;
	int size, idx=0;
	unsigned char tmpbuf[200], tmp1[100];

	unsigned short AuthType = 0;
	unsigned short EncryType = 0;
	
	if (include_cred)
		idx = 4;

	if (pCtx->config_state == CONFIG_STATE_UNCONFIGURED) {
		// auto-generated setting										
		generate_random(tmpbuf, (MAX_SSID_LEN/2)+BYTE_LEN_256B);
		memcpy(&tmpbuf[MAX_SSID_LEN/2-2], &pCtx->our_addr[ETHER_ADDRLEN-2], 2);
		convert_bin_to_str(tmpbuf, MAX_SSID_LEN/2, tmp1);

#ifndef CONFIG_RTL8186_KB
#ifdef CONFIG_RTL8186_KLD_REPEATER
		memset(pCtx->SSID, '\0', MAX_SSID_LEN+1);
		strcpy(pCtx->SSID, "dlink");
		strncat(pCtx->SSID, tmp1+(MAX_SSID_LEN-4), 4);	
#else
		memset(pCtx->SSID, '\0', MAX_SSID_LEN+1);
		strcpy(pCtx->SSID, "WPS");
		strncat(pCtx->SSID, tmp1, 6);
		strncat(pCtx->SSID, tmp1+(MAX_SSID_LEN-4), 4);	
#endif
		DEBUG_PRINT("auto-generated SSID: %s\n", pCtx->SSID);
#endif
		convert_bin_to_str(&tmpbuf[MAX_SSID_LEN/2], BYTE_LEN_256B, tmp1);
		
		memcpy(pCtx->network_key, tmp1, 22);
		pCtx->network_key[22] = '\0';
		pCtx->network_key_len = 22;
		DEBUG_PRINT("auto-generated netwok-key: %s\n", pCtx->network_key);			
		
		//self-generated WPA2Mixed Tkip+AES
		if (pCtx->is_ap) {
#ifdef CONFIG_RTL867x
			pCtx->auth_type_flash = AUTH_WPA2PSK;
			pCtx->encrypt_type_flash = ENCRYPT_AES; 
			pCtx->auth_type = AUTH_WPA2PSK;
			pCtx->encrypt_type = ENCRYPT_AES; 	
#else
			pCtx->auth_type_flash = (AUTH_WPAPSK | AUTH_WPA2PSK);
			pCtx->encrypt_type_flash = (ENCRYPT_TKIP | ENCRYPT_AES);
			pCtx->auth_type = AUTH_WPA2PSK;
			pCtx->encrypt_type = ENCRYPT_AES;	
#endif
		}
		else {
			pCtx->auth_type_flash = AUTH_WPA2PSK;
			pCtx->encrypt_type_flash = ENCRYPT_AES;
			pCtx->auth_type = AUTH_WPA2PSK;
			pCtx->encrypt_type = ENCRYPT_AES;
		}

		pSta->invoke_security_gen = 1;	
	}

// Set flag to notify web server to sync flash setting
	if (pCtx->is_ap && pCtx->config_state == CONFIG_STATE_CONFIGURED) {
		//andrew sprintf(tmpbuf, "flash set %s WSC_CONFIGBYEXTREG %d\n", pCtx->wlan_interface_name, MANUAL_SETTING_TO_ENROLLEE);
		// andrew system(tmpbuf);
		signal_webs(SYNC_FLASH_PARAMETER);
	}
//--------------------------- david+2007-01-26

#ifdef TEST_FOR_MULTIPLE_CREDENTIAL
	unsigned long start= (unsigned long)out;

	byteVal = 1;
	pMsg = add_tlv(&out[idx], TAG_NETWORK_INDEX, 1, &byteVal);
	pMsg = add_tlv(pMsg, TAG_SSID, strlen("Sean-WPA2"), "Sean-WPA2");
	shortVal = AUTH_WPA2PSK;
	pMsg = add_tlv(pMsg, TAG_AUTH_TYPE, 2, &shortVal);
	shortVal = ENCRYPT_AES;
	pMsg = add_tlv(pMsg, TAG_ENCRYPT_TYPE, 2, &shortVal);
	byteVal = 1;
	pMsg = add_tlv(pMsg, TAG_NETWORK_KEY_INDEX, 1, &byteVal);
	pMsg = add_tlv(pMsg, TAG_NETWORK_KEY, pCtx->network_key_len, pCtx->network_key);
	pMsg = add_tlv(pMsg, TAG_MAC_ADDRESS, ETHER_ADDRLEN, pSta->msg_addr);
	int size1 = (int)(((unsigned long)pMsg) - ((unsigned long)out));
	if (include_cred) {
		shortVal = ntohs(TAG_CREDENTIAL);
		memcpy(out, &shortVal, 2);

		shortVal = ntohs(size1-4);
		memcpy(&out[2], &shortVal, 2);
	}

	out = pMsg;
	byteVal = 2;
	pMsg = add_tlv(&out[idx], TAG_NETWORK_INDEX, 1, &byteVal);
	pMsg = add_tlv(pMsg, TAG_SSID, strlen("Sean-WPA3"), "Sean-WPA3");
	shortVal = AUTH_WPA2PSK;
	pMsg = add_tlv(pMsg, TAG_AUTH_TYPE, 2, &shortVal);
	shortVal = ENCRYPT_AES;
	pMsg = add_tlv(pMsg, TAG_ENCRYPT_TYPE, 2, &shortVal);
	byteVal = 1;
	pMsg = add_tlv(pMsg, TAG_NETWORK_KEY_INDEX, 1, &byteVal);
	pMsg = add_tlv(pMsg, TAG_NETWORK_KEY, pCtx->network_key_len, pCtx->network_key);
	pMsg = add_tlv(pMsg, TAG_MAC_ADDRESS, ETHER_ADDRLEN, pSta->msg_addr);
	int size2 = (int)(((unsigned long)pMsg) - ((unsigned long)out));
	if (include_cred) {
		shortVal = ntohs(TAG_CREDENTIAL);
		memcpy(out, &shortVal, 2);

		shortVal = ntohs(size2-4);
		memcpy(&out[2], &shortVal, 2);
	}

	out = pMsg;
	byteVal = 3;
	pMsg = add_tlv(&out[idx], TAG_NETWORK_INDEX, 1, &byteVal);
#else
	byteVal = 1;
	pMsg = add_tlv(&out[idx], TAG_NETWORK_INDEX, 1, &byteVal);
#endif
	pMsg = add_tlv(pMsg, TAG_SSID, strlen(pCtx->SSID), pCtx->SSID);
	TX_DEBUG("SSID=%s\n",pCtx->SSID);

	AuthType = (pSta->auth_type_flags & pCtx->auth_type_flash);
	if (AuthType) {
		if (AuthType & AUTH_WPA2PSK){
			AuthType = AUTH_WPA2PSK;
		}else if (AuthType & AUTH_WPAPSK){
			AuthType = AUTH_WPAPSK;
		}else if (AuthType & AUTH_OPEN){
			AuthType = AUTH_OPEN;
		}else if (AuthType & AUTH_SHARED){
			AuthType = AUTH_SHARED;
		}else {
			AuthType = pCtx->auth_type;
		}
		TX_DEBUG("Assign auth_type=0x%02X\n", AuthType);
	}
	else
	{	/*worse case , the auth type from STA is wrong , we need guess or use default*/ 
		//to do
		//if ((pSta->auth_type_flags & AUTH_WPA) &&  (pCtx->auth_type_flash & AUTH_WPAPSK))
		//	AuthType = AUTH_WPAPSK;
		
		if ((pSta->auth_type_flags & AUTH_WPA2) && (pCtx->auth_type_flash & AUTH_WPA2PSK))
			AuthType = AUTH_WPA2PSK;
		else { // for Intel SDK
			if (pCtx->auth_type_flash & AUTH_WPA2PSK){
				AuthType = AUTH_WPA2PSK;
			}else if (pCtx->auth_type_flash & AUTH_WPAPSK){
				AuthType = AUTH_WPAPSK;
			}else{
				AuthType = pCtx->auth_type;
			}
		}
		TX_DEBUG("Assign auth_type=0x%02X\n", AuthType);
	}
	
	EncryType = (pSta->encrypt_type_flags & pCtx->encrypt_type_flash);
	if (EncryType) {
		if (EncryType & ENCRYPT_AES){
			EncryType = ENCRYPT_AES;
		}else if (EncryType & ENCRYPT_TKIP){
			EncryType = ENCRYPT_TKIP;
		}else if (EncryType & ENCRYPT_NONE){
			EncryType = ENCRYPT_NONE;
		}else if (EncryType & ENCRYPT_WEP){
			EncryType = ENCRYPT_WEP;
		}else {
			EncryType = pCtx->encrypt_type;
			DEBUG_ERR("%s %d assign encrypt_type=%d\n", __FUNCTION__, __LINE__, EncryType);
		}
	}
	else {
		EncryType = pCtx->encrypt_type;
		DEBUG_ERR("%s %d assign encrypt_type=%d\n", __FUNCTION__, __LINE__, EncryType);
	}
	
#ifndef WPA2_MIXED_2MODE_ONLY
	/*if ap is under mixed mode*/ 
	if(pCtx->mixedmode){
		if(((pSta->auth_type_flags & AUTH_WPA2PSK) && (pSta->encrypt_type_flags & ENCRYPT_AES))
			&& (pCtx->mixedmode & WSC_WPA2_AES))
		{
			AuthType=AUTH_WPA2PSK;
			EncryType=ENCRYPT_AES;			
		}else if(((pSta->auth_type_flags & AUTH_WPAPSK) && (pSta->encrypt_type_flags & ENCRYPT_TKIP))
			&& (pCtx->mixedmode & WSC_WPA_TKIP)){
			AuthType=AUTH_WPAPSK;
			EncryType=ENCRYPT_TKIP;			
		}
		else if(((pSta->auth_type_flags & AUTH_WPAPSK) && (pSta->encrypt_type_flags & ENCRYPT_AES))
			&& (pCtx->mixedmode & WSC_WPA_AES)){
			AuthType=AUTH_WPAPSK;
			EncryType=ENCRYPT_AES;
		}
		else if(((pSta->auth_type_flags & AUTH_WPA2PSK) && (pSta->encrypt_type_flags & ENCRYPT_TKIP))
			&& (pCtx->mixedmode & WSC_WPA2_TKIP)){
			AuthType=AUTH_WPA2PSK;
			EncryType=ENCRYPT_TKIP;
		}else{
			TX_DEBUG("can't support STA's security capability\n");	
		}				
	}
#else // WPA2_MIXED_2MODE_ONLY

	if((AuthType == AUTH_WPA2PSK) && (EncryType== ENCRYPT_TKIP)) {
		if((pSta->auth_type_flags & pCtx->auth_type_flash)) {
			if((pSta->auth_type_flags & pCtx->auth_type_flash & AUTH_WPAPSK))
				AuthType = AUTH_WPAPSK;
		}
		else{
			if((pSta->auth_type_flags & AUTH_WPA) &&  (pCtx->auth_type_flash & AUTH_WPAPSK))
				AuthType = AUTH_WPAPSK;
		}
	}
	else if((AuthType == AUTH_WPAPSK) && (EncryType== ENCRYPT_AES)) {
		if((pSta->encrypt_type_flags & pCtx->encrypt_type_flash & ENCRYPT_TKIP))
			EncryType = ENCRYPT_TKIP;
	}
#endif // !WPA2_MIXED_2MODE_ONLY

	pMsg = add_tlv(pMsg, TAG_AUTH_TYPE, 2, &AuthType);
	pMsg = add_tlv(pMsg, TAG_ENCRYPT_TYPE, 2, &EncryType);
	
	TX_DEBUG("Auth Type=0x%02X\n",AuthType);
	TX_DEBUG("Encrypt Type=%d\n",EncryType);
	show_auth_encry_help();

	// Fix IOT issue with Ralink dongle -----------
	//	byteVal = 1;
	if (EncryType == ENCRYPT_WEP && pCtx->wep_transmit_key)
		byteVal = pCtx->wep_transmit_key;
	else
		byteVal = 1;
	//---------------------------- 12-18-2008
	
	pMsg = add_tlv(pMsg, TAG_NETWORK_KEY_INDEX, 1, &byteVal);
	pMsg = add_tlv(pMsg, TAG_NETWORK_KEY, pCtx->network_key_len, pCtx->network_key);
	TX_DEBUG("key index=%d\n",byteVal);

	pMsg = add_tlv(pMsg, TAG_MAC_ADDRESS, ETHER_ADDRLEN, pSta->msg_addr);
	
	/*2011-0419 , WEP,Hex type Key Issue*/
	if (EncryType == ENCRYPT_WEP && pCtx->wep_transmit_key){
		pMsg = add_tlv(pMsg, TAG_WEP_TRANSMIT_KEY, 1, &pCtx->wep_transmit_key);

		convert_bin_to_str(pCtx->network_key,pCtx->network_key_len,tmp1);
		TX_DEBUG("network key(string by hex)=%s\n",tmp1);		

	}else{
		TX_DEBUG("network key(string)=%s\n",pCtx->network_key);
	}

	size = (int)(((unsigned long)pMsg) - ((unsigned long)out));

	if (include_cred) {
		shortVal = ntohs(TAG_CREDENTIAL);
		memcpy(out, &shortVal, 2);

		shortVal = ntohs(size-4);
		memcpy(&out[2], &shortVal, 2);
	}
#ifdef TEST_FOR_MULTIPLE_CREDENTIAL
	out = (char *)start;
	return size1+size2+size;
#else
	return size;
#endif
}

#ifdef FOR_DUAL_BAND		/*for wlan1 interface*/
static int add_credential_attr2(CTX_Tp pCtx, STA_CTX_Tp pSta, char *out, int include_cred)
{
	unsigned char byteVal, *pMsg;
	unsigned short shortVal;
	int size, idx=0;
	unsigned char tmpbuf[200], tmp1[100];

	unsigned short AuthType = 0;
	unsigned short EncryType = 0;
	
	
	if (include_cred)
		idx = 4;

	if (pCtx->config_state == CONFIG_STATE_UNCONFIGURED) {
		// auto-generated setting										
		generate_random(tmpbuf, (MAX_SSID_LEN/2)+BYTE_LEN_256B);
		memcpy(&tmpbuf[MAX_SSID_LEN/2-2], &pCtx->our_addr[ETHER_ADDRLEN-2], 2); // don't care our_addr or addr2 , plus
		convert_bin_to_str(tmpbuf, MAX_SSID_LEN/2, tmp1);

		memset(pCtx->SSID2, '\0', MAX_SSID_LEN+1);				
		strcpy(pCtx->SSID2, "WPS");
		strncat(pCtx->SSID2, tmp1, 6);
		strncat(pCtx->SSID2, tmp1+(MAX_SSID_LEN-4), 4);						

		DEBUG_PRINT("auto-generated SSID2: %s\n", pCtx->SSID2);

		// network key setting
		convert_bin_to_str(&tmpbuf[MAX_SSID_LEN/2], BYTE_LEN_256B, tmp1);		
		memcpy(pCtx->network_key2, tmp1, 22);
		pCtx->network_key2[22] = '\0';
		pCtx->network_key_len2 = 22;			
		DEBUG_PRINT("PSK length no assigned use default\n");
		DEBUG_PRINT("auto-generated netwok-key: %s\n", pCtx->network_key2);			

		//self-generated WPA2Mixed Tkip+AES
		if (pCtx->is_ap) {
#ifdef CONFIG_RTL867x
			pCtx->auth_type_flash2 = AUTH_WPA2PSK;
			pCtx->encrypt_type_flash2 = ENCRYPT_AES; 
			pCtx->auth_type2 = AUTH_WPA2PSK;
			pCtx->encrypt_type2 = ENCRYPT_AES; 
#else //CONFIG_RTL867x
			pCtx->auth_type_flash2 = (AUTH_WPAPSK | AUTH_WPA2PSK);
			pCtx->encrypt_type_flash2 = (ENCRYPT_TKIP | ENCRYPT_AES);
			pCtx->auth_type2 = AUTH_WPA2PSK;
			pCtx->encrypt_type2 = ENCRYPT_AES;

			if (pCtx->manual_key_type) {
				if (pCtx->manual_key_type == 1) {
					pCtx->auth_type2 = AUTH_WPAPSK;
					pCtx->encrypt_type2 = ENCRYPT_TKIP;					
					pCtx->auth_type_flash2 = AUTH_WPAPSK;
					pCtx->encrypt_type_flash2 = ENCRYPT_TKIP;					
				}
				else if (pCtx->manual_key_type == 2) {
					pCtx->auth_type2 = AUTH_WPA2PSK;
					pCtx->encrypt_type2 = ENCRYPT_AES;	
					pCtx->auth_type_flash2 = AUTH_WPA2PSK;
					pCtx->encrypt_type_flash2 = ENCRYPT_AES;					
				}
				else if (pCtx->manual_key_type == 3) {
					pCtx->auth_type2 = AUTH_WPA2PSKMIXED;
					pCtx->encrypt_type2 = ENCRYPT_TKIPAES;
					pCtx->auth_type_flash2 = (AUTH_WPAPSK | AUTH_WPA2PSK);
					pCtx->encrypt_type_flash2 = (ENCRYPT_TKIP | ENCRYPT_AES);					
				}
			}
#endif //CONFIG_RTL867x
		}
		else {
			pCtx->auth_type_flash2 = AUTH_WPAPSK;
			pCtx->encrypt_type_flash2 = ENCRYPT_TKIP;
			pCtx->auth_type2 = AUTH_WPAPSK;
			pCtx->encrypt_type2 = ENCRYPT_TKIP;
		}

		pSta->invoke_security_gen = 1;	
	}

// Set flag to notify web server to sync flash setting
	if (pCtx->is_ap && pCtx->config_state == CONFIG_STATE_CONFIGURED) {
		//andrew sprintf(tmpbuf, "flash set %s WSC_CONFIGBYEXTREG %d\n", pCtx->wlan_interface_name, MANUAL_SETTING_TO_ENROLLEE);
		// andrew system(tmpbuf);
		signal_webs(SYNC_FLASH_PARAMETER);
	}
//--------------------------- david+2007-01-26

#ifdef TEST_FOR_MULTIPLE_CREDENTIAL
	unsigned long start= (unsigned long)out;

	byteVal = 1;
	pMsg = add_tlv(&out[idx], TAG_NETWORK_INDEX, 1, &byteVal);
	pMsg = add_tlv(pMsg, TAG_SSID, strlen("Sean-WPA2"), "Sean-WPA2");
	shortVal = AUTH_WPA2PSK;
	pMsg = add_tlv(pMsg, TAG_AUTH_TYPE, 2, &shortVal);
	shortVal = ENCRYPT_AES;
	pMsg = add_tlv(pMsg, TAG_ENCRYPT_TYPE, 2, &shortVal);
	byteVal = 1;
	pMsg = add_tlv(pMsg, TAG_NETWORK_KEY_INDEX, 1, &byteVal);
	pMsg = add_tlv(pMsg, TAG_NETWORK_KEY, pCtx->network_key_len2, pCtx->network_key2);
	pMsg = add_tlv(pMsg, TAG_MAC_ADDRESS, ETHER_ADDRLEN, pSta->msg_addr);
	int size1 = (int)(((unsigned long)pMsg) - ((unsigned long)out));
	if (include_cred) {
		shortVal = ntohs(TAG_CREDENTIAL);
		memcpy(out, &shortVal, 2);

		shortVal = ntohs(size1-4);
		memcpy(&out[2], &shortVal, 2);
	}

	out = pMsg;
	byteVal = 2;
	pMsg = add_tlv(&out[idx], TAG_NETWORK_INDEX, 1, &byteVal);
	pMsg = add_tlv(pMsg, TAG_SSID, strlen("Sean-WPA3"), "Sean-WPA3");
	shortVal = AUTH_WPA2PSK;
	pMsg = add_tlv(pMsg, TAG_AUTH_TYPE, 2, &shortVal);
	shortVal = ENCRYPT_AES;
	pMsg = add_tlv(pMsg, TAG_ENCRYPT_TYPE, 2, &shortVal);
	byteVal = 1;
	pMsg = add_tlv(pMsg, TAG_NETWORK_KEY_INDEX, 1, &byteVal);
	pMsg = add_tlv(pMsg, TAG_NETWORK_KEY, pCtx->network_key_len2, pCtx->network_key2);
	pMsg = add_tlv(pMsg, TAG_MAC_ADDRESS, ETHER_ADDRLEN, pSta->msg_addr);
	int size2 = (int)(((unsigned long)pMsg) - ((unsigned long)out));
	if (include_cred) {
		shortVal = ntohs(TAG_CREDENTIAL);
		memcpy(out, &shortVal, 2);

		shortVal = ntohs(size2-4);
		memcpy(&out[2], &shortVal, 2);
	}

	out = pMsg;
	byteVal = 3;
	pMsg = add_tlv(&out[idx], TAG_NETWORK_INDEX, 1, &byteVal);
#else
	byteVal = 1;
	pMsg = add_tlv(&out[idx], TAG_NETWORK_INDEX, 1, &byteVal);
#endif

	pMsg = add_tlv(pMsg, TAG_SSID, strlen(pCtx->SSID2), pCtx->SSID2);
	TX_DEBUG("SSID2=%s\n",pCtx->SSID2);
	
	AuthType = (pSta->auth_type_flags & pCtx->auth_type_flash2);
	if (AuthType) {
		if (AuthType & AUTH_WPA2PSK){
			AuthType = AUTH_WPA2PSK;
		}else if (AuthType & AUTH_WPAPSK){
			AuthType = AUTH_WPAPSK;
		}else if (AuthType & AUTH_OPEN){
			AuthType = AUTH_OPEN;
		}else if (AuthType & AUTH_SHARED){
			AuthType = AUTH_SHARED;
		}else {
			AuthType = pCtx->auth_type2;
		}
		TX_DEBUG("Assign auth_type=0x%02X\n", AuthType);
	}
	else {
		if ((pSta->auth_type_flags & AUTH_WPA2) && (pCtx->auth_type_flash2 & AUTH_WPA2PSK))
			AuthType = AUTH_WPA2PSK;
		else { // for Intel SDK
			if (pCtx->auth_type_flash2 & AUTH_WPA2PSK)
				AuthType = AUTH_WPA2PSK;
			else if (pCtx->auth_type_flash2 & AUTH_WPAPSK)
				AuthType = AUTH_WPAPSK;
			else
				AuthType = pCtx->auth_type2;
		}
		TX_DEBUG("Assign auth_type=0x%02X\n", AuthType);
	}
	
	EncryType = (pSta->encrypt_type_flags & pCtx->encrypt_type_flash2);
	if (EncryType) {
		if (EncryType & ENCRYPT_AES){
			EncryType = ENCRYPT_AES;
		}else if (EncryType & ENCRYPT_TKIP){
			EncryType = ENCRYPT_TKIP;
		}else if (EncryType & ENCRYPT_NONE){
			EncryType = ENCRYPT_NONE;
		}else if (EncryType & ENCRYPT_WEP){
			EncryType = ENCRYPT_WEP;
		}else {
			EncryType = pCtx->encrypt_type2;
			DEBUG_ERR("%s %d assign encrypt_type=%d\n", __FUNCTION__, __LINE__, EncryType);
		}
	}
	else {
		EncryType = pCtx->encrypt_type2;
		DEBUG_ERR("%s %d assign encrypt_type=%d\n", __FUNCTION__, __LINE__, EncryType);
	}
	
#ifndef WPA2_MIXED_2MODE_ONLY
	/*if ap is under mixed mode*/ 
	if(pCtx->mixedmode2){
		if(((pSta->auth_type_flags & AUTH_WPA2PSK) && (pSta->encrypt_type_flags & ENCRYPT_AES))
			&& (pCtx->mixedmode2 & WSC_WPA2_AES))
		{
			AuthType=AUTH_WPA2PSK;
			EncryType=ENCRYPT_AES;			
		}else if(((pSta->auth_type_flags & AUTH_WPAPSK) && (pSta->encrypt_type_flags & ENCRYPT_TKIP))
			&& (pCtx->mixedmode2 & WSC_WPA_TKIP)){
			AuthType=AUTH_WPAPSK;
			EncryType=ENCRYPT_TKIP;			
		}
		else if(((pSta->auth_type_flags & AUTH_WPAPSK) && (pSta->encrypt_type_flags & ENCRYPT_AES))
			&& (pCtx->mixedmode2 & WSC_WPA_AES)){
			AuthType=AUTH_WPAPSK;
			EncryType=ENCRYPT_AES;
		}
		else if(((pSta->auth_type_flags & AUTH_WPA2PSK) && (pSta->encrypt_type_flags & ENCRYPT_TKIP))
			&& (pCtx->mixedmode2 & WSC_WPA2_TKIP)){
			AuthType=AUTH_WPA2PSK;
			EncryType=ENCRYPT_TKIP;
		}else{
			TX_DEBUG("can't support STA's security capability\n");	
		}				
	}
#else // WPA2_MIXED_2MODE_ONLY

	if((AuthType == AUTH_WPA2PSK) && (EncryType== ENCRYPT_TKIP)) {
		if((pSta->auth_type_flags & pCtx->auth_type_flash2)) {
			if((pSta->auth_type_flags & pCtx->auth_type_flash2 & AUTH_WPAPSK))
				AuthType = AUTH_WPAPSK;
		}
		else{
			if((pSta->auth_type_flags & AUTH_WPA) &&  (pCtx->auth_type_flash2 & AUTH_WPAPSK))
				AuthType = AUTH_WPAPSK;
		}
	}
	else if((AuthType == AUTH_WPAPSK) && (EncryType== ENCRYPT_AES)) {
		if((pSta->encrypt_type_flags & pCtx->encrypt_type_flash2 & ENCRYPT_TKIP))
			EncryType = ENCRYPT_TKIP;
	}
#endif // !WPA2_MIXED_2MODE_ONLY

	pMsg = add_tlv(pMsg, TAG_AUTH_TYPE, 2, &AuthType);
	pMsg = add_tlv(pMsg, TAG_ENCRYPT_TYPE, 2, &EncryType);
	
	TX_DEBUG("Auth Type=0x%02X\n",AuthType);
	TX_DEBUG("Encrypt Type=%d\n",EncryType);
	show_auth_encry_help();

	// Fix IOT issue with Ralink dongle -----------
	//	byteVal = 1;
	if (EncryType == ENCRYPT_WEP && pCtx->wep_transmit_key2)
		byteVal = pCtx->wep_transmit_key2;
	else
		byteVal = 1;
	//---------------------------- 12-18-2008

	pMsg = add_tlv(pMsg, TAG_NETWORK_KEY_INDEX, 1, &byteVal);
	pMsg = add_tlv(pMsg, TAG_NETWORK_KEY, pCtx->network_key_len2, pCtx->network_key2);
	TX_DEBUG("key index2=%d\n",byteVal);


	#if 0 // for Intel SDK
	pMsg = add_tlv(pMsg, TAG_MAC_ADDRESS, ETHER_ADDRLEN, pSta->addr);
	#else
	pMsg = add_tlv(pMsg, TAG_MAC_ADDRESS, ETHER_ADDRLEN, pSta->msg_addr);
	#endif
	
	if (EncryType == ENCRYPT_WEP && pCtx->wep_transmit_key2){
		pMsg = add_tlv(pMsg, TAG_WEP_TRANSMIT_KEY, 1, &pCtx->wep_transmit_key2);
		convert_bin_to_str(pCtx->network_key2,pCtx->network_key_len2,tmp1);
		TX_DEBUG("network key2(string by hex)=%s\n",tmp1);		
	}else{
		TX_DEBUG("network key2(string)=%s\n",pCtx->network_key2);	
	}

	size = (int)(((unsigned long)pMsg) - ((unsigned long)out));

	if (include_cred) {
		shortVal = ntohs(TAG_CREDENTIAL);
		memcpy(out, &shortVal, 2);

		shortVal = ntohs(size-4);
		memcpy(&out[2], &shortVal, 2);
	}
#ifdef TEST_FOR_MULTIPLE_CREDENTIAL
	out = (char *)start;
	return size1+size2+size;
#else
	return size;
#endif
}
#endif //FOR_DUAL_BAND

int send_wlan(CTX_Tp pCtx, unsigned char *data, int size)
{
	int n;

	DBFENTER;

#ifdef DEBUG
	if (pCtx->debug2)
		debug_out("wlan sent-out data", data, size);
#endif

#ifdef FOR_DUAL_BAND
	if(pCtx->is_ap){
		if(pCtx->InterFaceComeIn == COME_FROM_WLAN1){
			//DEBUG_PRINT2("Send out wlan1 \n");
			if ((n = send(pCtx->socket2, data, size, 0)) < 0) {
				TX_DEBUG("send out wlan1 fail\n");
				return -1;
			}
		}
		else if(pCtx->InterFaceComeIn == COME_FROM_WLAN0){
			//DEBUG_PRINT("\nSend out wlan0\n");
			if ((n = send(pCtx->socket, data, size, 0)) < 0) {
				TX_DEBUG("send out wlan0 fail\n");
				return -1;
			}
		}
		else	{
			//DEBUG_PRINT("\n Default Send out wlan0\n");
			if ((n = send(pCtx->socket, data, size, 0)) < 0) {
				TX_DEBUG("send to wlan0 fail\n");			
				return -1;
			}		
		}
	}
	else{
		if ((n = send(pCtx->socket, data, size, 0)) < 0) {
			TX_DEBUG("wlan0 send out fail \n");
			return -1;
		}
	}
#else //FOR_DUAL_BAND
	if ((n = send(pCtx->socket, data, size, 0)) < 0) {
		perror("send_wlan failed!");
		return -1;
	}
#endif //FOR_DUAL_BAND

	return 0;	
}

int send_eap_reqid(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	struct eapol_t *eapol;
	struct eap_t *eap;
	unsigned char *packet;
	struct ethernet_t *eth_hdr;
	struct eap_rr_t * eaprr;
	int size;

	DBFENTER;

	size = ETHER_HDRLEN + sizeof(struct eapol_t) + sizeof(struct eap_t ) + sizeof(struct eap_rr_t);
	memset(pSta->tx_buffer, '\0', size);
	packet = pSta->tx_buffer;

	eth_hdr = (struct ethernet_t *)packet;
	memcpy(eth_hdr->ether_dhost, pSta->addr, ETHER_ADDRLEN);

#ifdef FOR_DUAL_BAND		
	if(pCtx->InterFaceComeIn == COME_FROM_WLAN1){
		memcpy(eth_hdr->ether_shost, pCtx->our_addr2, ETHER_ADDRLEN);		
	}
	else if(pCtx->InterFaceComeIn == COME_FROM_WLAN0){
		memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);		
	}
	else	{
		memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);		
	}
#else //FOR_DUAL_BAND
	memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);
#endif //FOR_DUAL_BAND
	eth_hdr->ether_type = htons(ETHER_EAPOL_TYPE);

	eapol = (struct eapol_t *)(packet + ETHER_HDRLEN);
	eapol->protocol_version = EAPOL_VER;
	eapol->packet_type = EAPOL_EAPPKT;
	eapol->packet_body_length = htons(EAP_HDRLEN  + sizeof(struct eap_rr_t));

	eap = (struct eap_t *)(((unsigned char *)eapol) + EAPOL_HDRLEN);
	eap->code =  EAP_REQUEST;
	eap->identifier = pSta->eap_reqid;	
	
	eap->length = htons(EAP_HDRLEN + sizeof(struct eap_rr_t));

	eaprr = (struct eap_rr_t *)(((unsigned char *)eapol) + EAPOL_HDRLEN  + EAP_HDRLEN);
	eaprr->type =  EAP_TYPE_IDENTITY;

	DEBUG_PRINT("\n>> Sending EAP REQUEST / Identity packet\n");

	pSta->tx_size = size;

	return send_wlan(pCtx, pSta->tx_buffer, pSta->tx_size);	
}

int send_wsc_done(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	struct eapol_t *eapol;
	struct eap_t *eap;
	struct eap_wsc_t *wsc;
	int size;
	unsigned char *pMsg, *pMsgStart, byteVal;

	DBFENTER;
#ifdef SUPPORT_UPNP
	if (pSta->used & IS_UPNP_CONTROL_POINT)
		pMsg = pSta->tx_buffer;
	else
#endif
	{
		build_wsc_msg_header(pCtx, pSta, WSC_OP_DONE, &eapol, &eap, &wsc);
		pMsg =	((unsigned char *)wsc) + sizeof(struct eap_wsc_t);
	}
	pMsgStart = pMsg;

	byteVal = WSC_VER;
	pMsg = add_tlv(pMsg, TAG_VERSION, 1, (void *)&byteVal);
	byteVal = MSG_TYPE_DONE;	
	pMsg = add_tlv(pMsg, TAG_MSG_TYPE, 1, (void *)&byteVal);	
	pMsg = add_tlv(pMsg, TAG_EROLLEE_NONCE, NONCE_LEN, (void *)pSta->nonce_enrollee);
	pMsg = add_tlv(pMsg, TAG_REGISTRAR_NONCE, NONCE_LEN, (void *)pSta->nonce_registrar);
#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2)
		pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, 6, (void *)WSC_VENDOR_V2);
#endif

	//================================================================	



	size = (int)(((unsigned long)pMsg) - ((unsigned long)pMsgStart));
	
#ifdef SUPPORT_UPNP
	pSta->last_tx_msg_size = size;
	if (!(pSta->used & IS_UPNP_CONTROL_POINT)) 
#endif
	{
		eapol->packet_body_length = htons(EAP_HDRLEN + sizeof(*wsc) + size);
		eap->length = htons(EAP_HDRLEN + sizeof(*wsc) + size);
		pSta->tx_size = ETHER_HDRLEN + EAPOL_HDRLEN + EAP_HDRLEN + sizeof(*wsc) + size;
	}

        report_WPS_STATUS(PROTOCOL_SUCCESS);
	_DEBUG_PRINT("\n>> Sending EAP WSC_Done\n");

#ifdef SUPPORT_UPNP
	if (pSta->used & IS_UPNP_CONTROL_POINT)
		return 0;
	else
#endif
		return send_wlan(pCtx, pSta->tx_buffer, pSta->tx_size);	
}

int send_wsc_start(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	struct eapol_t *eapol;
	struct eap_t *eap;
	unsigned char *packet;
	struct ethernet_t *eth_hdr;
	struct eap_wsc_t *wsc;
	int size;

	DBFENTER;	

	size = ETHER_HDRLEN + sizeof(struct eapol_t) + sizeof(struct eap_t) + sizeof(struct eap_wsc_t);
	memset(pSta->tx_buffer, '\0', size);
	packet = pSta->tx_buffer;

	eth_hdr = (struct ethernet_t *)packet;
	memcpy(eth_hdr->ether_dhost, pSta->addr, ETHER_ADDRLEN);

#ifdef FOR_DUAL_BAND		
	if(pCtx->InterFaceComeIn == COME_FROM_WLAN1){
		memcpy(eth_hdr->ether_shost, pCtx->our_addr2, ETHER_ADDRLEN);		
	}
	else if(pCtx->InterFaceComeIn == COME_FROM_WLAN0){
		memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);		
	}
	else	{
		memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);		
	}
#else //FOR_DUAL_BAND
	memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);
#endif //FOR_DUAL_BAND

	eth_hdr->ether_type = htons(ETHER_EAPOL_TYPE);

	eapol = (struct eapol_t *)(packet + ETHER_HDRLEN);
	eapol->protocol_version = EAPOL_VER;
	eapol->packet_type = EAPOL_EAPPKT;
	eapol->packet_body_length = htons(EAP_HDRLEN  + sizeof(struct eap_wsc_t));

	eap = (struct eap_t *)(((unsigned char *)eapol) + EAPOL_HDRLEN);
	eap->code =  EAP_REQUEST;

	if (pCtx->is_ap){
		pSta->eap_reqid+=1;
		DEBUG_PRINT2("AP's identifier ++ to:0x%x\n",pSta->eap_reqid);		
	}
	
	eap->identifier = pSta->eap_reqid;
	eap->length = htons(EAP_HDRLEN + sizeof(struct eap_wsc_t));

	wsc = (struct eap_wsc_t *)(((unsigned char *)eap) + EAP_HDRLEN);
	wsc->type =  EAP_TYPE_EXPANDED;
	memcpy(wsc->vendor_id, WSC_VENDOR_ID, 3);
	wsc->vendor_type = htonl(WSC_VENDOR_TYPE);
	wsc->op_code = WSC_OP_START;

	pSta->tx_size = size;

	report_WPS_STATUS(SEND_EAP_START);
	_DEBUG_PRINT("\n>> Sending EAP WSC_Start\n");

	return send_wlan(pCtx, pSta->tx_buffer, pSta->tx_size);	
}


#ifdef WPS2DOTX
//for support  EAP_FRAGMENT ,  EAP_REASSEMBLY
int send_wsc_frag_ack(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	struct eapol_t *eapol;
	struct eap_t *eap;
	unsigned char *packet;
	struct ethernet_t *eth_hdr;
	struct eap_wsc_t *wsc;
	int size;

	DBFENTER;	

	size = ETHER_HDRLEN + sizeof(struct eapol_t) + sizeof(struct eap_t) + sizeof(struct eap_wsc_t);
	memset(pSta->tx_buffer, '\0', size);
	packet = pSta->tx_buffer;

	eth_hdr = (struct ethernet_t *)packet;
	memcpy(eth_hdr->ether_dhost, pSta->addr, ETHER_ADDRLEN);

#ifdef FOR_DUAL_BAND		
	if(pCtx->InterFaceComeIn == COME_FROM_WLAN1){
		
		memcpy(eth_hdr->ether_shost, pCtx->our_addr2, ETHER_ADDRLEN);		
	}
	else if(pCtx->InterFaceComeIn == COME_FROM_WLAN0){
		
		memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);		
	}
	else	{
		memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);		
	}

#else
	memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);	
#endif
	eth_hdr->ether_type = htons(ETHER_EAPOL_TYPE);

	eapol = (struct eapol_t *)(packet + ETHER_HDRLEN);
	eapol->protocol_version = EAPOL_VER;
	eapol->packet_type = EAPOL_EAPPKT;
	eapol->packet_body_length = htons(EAP_HDRLEN  + sizeof(struct eap_wsc_t));

	eap = (struct eap_t *)(((unsigned char *)eapol) + EAPOL_HDRLEN);

	if(pCtx->is_ap)
		eap->code =  EAP_REQUEST;	
	else
		eap->code =  EAP_RESPONSE;	


	
	// !!!! ok  , fix 4.2.10 test item 
	if (pCtx->is_ap){
		pSta->eap_reqid+=1;
		DEBUG_PRINT2("AP's identifier ++ to:0x%x\n",pSta->eap_reqid);		
	}

	
	eap->identifier = pSta->eap_reqid;
	eap->length = htons(EAP_HDRLEN + sizeof(struct eap_wsc_t));

	wsc = (struct eap_wsc_t *)(((unsigned char *)eap) + EAP_HDRLEN);
	wsc->type =  EAP_TYPE_EXPANDED;
	memcpy(wsc->vendor_id, WSC_VENDOR_ID, 3);
	wsc->vendor_type = htonl(WSC_VENDOR_TYPE);
	wsc->op_code = WSC_OP_FRAG_ACK;

	pSta->tx_size = size;
	_DEBUG_PRINT("\n>> Sending EAP WSC_Frag_Ack\n");

	return send_wlan(pCtx, pSta->tx_buffer, pSta->tx_size );	
}
#endif

int send_eap_fail(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	struct eapol_t *eapol;
	unsigned char *packet;
	struct eap_t *eap;	
	struct ethernet_t *eth_hdr;
	int size;

	DBFENTER;

	size = ETHER_HDRLEN + sizeof(struct eapol_t) + sizeof(struct eap_t);
	memset(pSta->tx_buffer, '\0', size);
	packet = pSta->tx_buffer;

	eth_hdr = (struct ethernet_t *)packet;
	memcpy(eth_hdr->ether_dhost, pSta->addr, ETHER_ADDRLEN);

#ifdef FOR_DUAL_BAND		
	if(pCtx->InterFaceComeIn == COME_FROM_WLAN1){
		
		memcpy(eth_hdr->ether_shost, pCtx->our_addr2, ETHER_ADDRLEN);
	}
	else if(pCtx->InterFaceComeIn == COME_FROM_WLAN0){
		
		memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);
	}
	else {
		memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);		
	}
#else //FOR_DUAL_BAND
	memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);
#endif //FOR_DUAL_BAND

	eth_hdr->ether_type = htons(ETHER_EAPOL_TYPE);

	eapol = (struct eapol_t *)(packet + ETHER_HDRLEN);
	eapol->protocol_version = EAPOL_VER;
	eapol->packet_type = EAPOL_EAPPKT;
	eapol->packet_body_length = htons(EAP_HDRLEN);

	eap = (struct eap_t *)(((unsigned char *)eapol) + EAPOL_HDRLEN);
	eap->code =  EAP_FAIL;

	if (pCtx->is_ap){
		pSta->eap_reqid+=1;
		DEBUG_PRINT2("AP's identifier ++ to:0x%x\n",pSta->eap_reqid);		
	}
	
	eap->identifier = pSta->eap_reqid;
	eap->length = htons(EAP_HDRLEN);

        report_WPS_STATUS(WSC_EAP_FAIL);
	_DEBUG_PRINT("\n>> Sending EAP-Fail packet\n");

	pSta->tx_size = size;

	send_wlan(pCtx, pSta->tx_buffer, pSta->tx_size);	

	if (pSta->invoke_security_gen) {
#ifdef FOR_DUAL_BAND
		if(pCtx->InterFaceComeIn == COME_FROM_WLAN0)
			pCtx->wait_reinit = write_param_to_flash(pCtx, 1);
		else if(pCtx->InterFaceComeIn == COME_FROM_WLAN1)
			pCtx->wait_reinit = write_param_to_flash2(pCtx, 1);
#else //FOR_DUAL_BAND
		pCtx->wait_reinit = write_param_to_flash(pCtx, 1);
#endif //FOR_DUAL_BAND
		pSta->invoke_security_gen = 0;
	}
	
	if (pCtx->wait_reinit)
		pCtx->wait_reboot = WAIT_REBOOT_TIME;

	return 0;
}

int send_wsc_nack(CTX_Tp pCtx, STA_CTX_Tp pSta, int err_code)
{
	struct eapol_t *eapol;
	struct eap_t *eap;
	struct eap_wsc_t *wsc;
	int size;
	unsigned char *pMsg, byteVal, *pMsgStart;
	unsigned short shortVal;

	DBFENTER;

#ifdef SUPPORT_UPNP
	if (pSta->used & IS_UPNP_CONTROL_POINT) 
		pMsg = pSta->tx_buffer;
	else
#endif
	{
		build_wsc_msg_header(pCtx, pSta, WSC_OP_NACK, &eapol, &eap, &wsc);
		pMsg =	((unsigned char *)wsc) + sizeof(struct eap_wsc_t);
	}
	pMsgStart = pMsg;

	byteVal = WSC_VER;
	pMsg = add_tlv(pMsg, TAG_VERSION, 1, (void *)&byteVal);
	byteVal = MSG_TYPE_NACK;
	pMsg = add_tlv(pMsg, TAG_MSG_TYPE, 1, (void *)&byteVal);	
	pMsg = add_tlv(pMsg, TAG_EROLLEE_NONCE, NONCE_LEN, (void *)pSta->nonce_enrollee);
	pMsg = add_tlv(pMsg, TAG_REGISTRAR_NONCE, NONCE_LEN, (void *)pSta->nonce_registrar);
	shortVal = htons(err_code);
	pMsg = add_tlv(pMsg, TAG_CONFIG_ERR, 2, (void *)&shortVal);
#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2)
		pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, 6, (void *)WSC_VENDOR_V2);
#endif
	//================================================================	


	
	size = (int)(((unsigned long)pMsg) - ((unsigned long)pMsgStart));
	pSta->last_tx_msg_size = size;
	
#ifdef SUPPORT_UPNP
	if (!(pSta->used & IS_UPNP_CONTROL_POINT)) 
#endif
	{
		eapol->packet_body_length = htons(EAP_HDRLEN + sizeof(*wsc) + size);
		eap->length = htons(EAP_HDRLEN + sizeof(*wsc) + size);
		pSta->tx_size = ETHER_HDRLEN + EAPOL_HDRLEN + EAP_HDRLEN + sizeof(*wsc) + size;
		_DEBUG_PRINT("\n>> Sending EAP WSC_NACK\n");
		return send_wlan(pCtx, pSta->tx_buffer, pSta->tx_size);	
	}
#ifdef SUPPORT_UPNP
	else {
		_DEBUG_PRINT("\n>> Sending UPnP WSC_NACK\n");
		return 0;
	}
#endif	
}

int send_wsc_ack(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	struct eapol_t *eapol;
	struct eap_t *eap;
	struct eap_wsc_t *wsc;
	int size;
	unsigned char *pMsg, byteVal, *pMsgStart;

	DBFENTER;

#ifdef SUPPORT_UPNP
	if (pSta->used & IS_UPNP_CONTROL_POINT) 
		pMsg = pSta->tx_buffer;
	else
#endif
	{
		build_wsc_msg_header(pCtx, pSta, WSC_OP_ACK, &eapol, &eap, &wsc);
		pMsg =	((unsigned char *)wsc) + sizeof(struct eap_wsc_t);
	}
	pMsgStart = pMsg;

	byteVal = WSC_VER;
	pMsg = add_tlv(pMsg, TAG_VERSION, 1, (void *)&byteVal);
	byteVal = MSG_TYPE_ACK;
	pMsg = add_tlv(pMsg, TAG_MSG_TYPE, 1, (void *)&byteVal);
	pMsg = add_tlv(pMsg, TAG_EROLLEE_NONCE, NONCE_LEN, (void *)pSta->nonce_enrollee);
	pMsg = add_tlv(pMsg, TAG_REGISTRAR_NONCE, NONCE_LEN, (void *)pSta->nonce_registrar);


#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2)
		pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, 6, (void *)WSC_VENDOR_V2);
#endif
	//================================================================	



	size = (int)(((unsigned long)pMsg) - ((unsigned long)pMsgStart));

#ifdef SUPPORT_UPNP
	if (!(pSta->used & IS_UPNP_CONTROL_POINT)) 
#endif
	{
		eapol->packet_body_length = htons(EAP_HDRLEN + sizeof(*wsc) + size);
		eap->length = htons(EAP_HDRLEN + sizeof(*wsc) + size);
		pSta->tx_size = ETHER_HDRLEN + EAPOL_HDRLEN + EAP_HDRLEN + sizeof(*wsc) + size;
		_DEBUG_PRINT("\n>> Sending EAP WSC_ACK\n");
		return send_wlan(pCtx, pSta->tx_buffer, pSta->tx_size);
	}
#ifdef SUPPORT_UPNP
	else {
		pSta->last_tx_msg_size = size;
		_DEBUG_PRINT("\n>> Sending UPnP WSC_ACK\n");
		return 0;
	}
#endif
}


#ifdef WPS2DOTX

#define HEAD_LEN_a_WSC  (ETHER_HDRLEN + EAPOL_HDRLEN + EAP_HDRLEN + 10)
#define WSC_HAD_LEN 10

/*
when tx a  message need to do  fragment then call send_frag_msg to do 

int times  = 1 for first fragment packet
int NextStat : when sent last eap fragment packet , psta->STATE WILL UPDATE TO "NextStat"


*/
int send_frag_msg(CTX_Tp pCtx, STA_CTX_Tp pSta ,  int NextStat , int times )
{
	//osize just  use for first time
	static int Frag_Count = 0;		
	static int lastMsgLen = 0;					
	static unsigned char* copyPrt=NULL;
	static int FragIndex = 0;
		
	unsigned short origTotalMsgLen;	
	unsigned char frag_buffer[1024];
	memset(frag_buffer,'\0',1024);	
	unsigned char* pMsgStart = NULL;	
	struct eap_t *eap=NULL;

	_DEBUG_PRINT("\n\n");

	
	if(times==1){	// from send_wsc_Mx()  caller

		/*to record this fragment need how many packet to send , 
		  every packet len = pCtx->EAP_frag_threshold*/
		Frag_Count = (pSta->sendToSta_MegTotalSize / pCtx->EAP_frag_threshold) + 1 ;

		/*to record the length of last fragment packet*/		
		lastMsgLen = (pSta->sendToSta_MegTotalSize % pCtx->EAP_frag_threshold) ;

		/*first one frag packet*/
		FragIndex = 1;
		
		origTotalMsgLen = htons(pSta->sendToSta_MegTotalSize) ;

		TX_DEBUG("EAP frag Threshold= %d\n",pCtx->EAP_frag_threshold);	
		TX_DEBUG("EAP count= %d , lastMsgLen=%d\n",Frag_Count , lastMsgLen);	
		TX_DEBUG("OrigTotalMsgLen= %d\n",origTotalMsgLen);			
		debug_out("Orig message data",  pSta->tx_buffer+HEAD_LEN_a_WSC , origTotalMsgLen);
	}	
						
	if(FragIndex==1){

		/*	First packet	*/ 

		//TX_DEBUG("send first fragment EAP \n");			
		pCtx->Feapol->packet_body_length = htons(EAP_HDRLEN + WSC_HAD_LEN + MSG_LEN_LEN + pCtx->EAP_frag_threshold);
		pCtx->Feap->length= htons(EAP_HDRLEN + WSC_HAD_LEN + MSG_LEN_LEN + pCtx->EAP_frag_threshold);
		pSta->tx_size= HEAD_LEN_a_WSC +MSG_LEN_LEN+pCtx->EAP_frag_threshold;
		pCtx->Fwsc->flags = (EAP_FR_MF | EAP_FR_LF);
								
		memcpy(frag_buffer , pSta->tx_buffer ,HEAD_LEN_a_WSC);
		memcpy(frag_buffer+HEAD_LEN_a_WSC , (void *)&origTotalMsgLen ,2);	 // insert message length
		memcpy(frag_buffer+HEAD_LEN_a_WSC+2 , (void *)(pSta->tx_buffer+HEAD_LEN_a_WSC) ,pCtx->EAP_frag_threshold);

		//TX_DEBUG("\n>> Sending EAP WSC_MSG (First Frag)\n");

		if(pCtx->debug){
			debug_out("first message data",  frag_buffer+HEAD_LEN_a_WSC+2 , pCtx->EAP_frag_threshold);
		}
		
		send_wlan(pCtx, frag_buffer, pSta->tx_size);
		copyPrt = pSta->tx_buffer + HEAD_LEN_a_WSC + pCtx->EAP_frag_threshold;
		//pSta->wait_frag_act = 1;
		pSta->state = NextStat ;
		TX_DEBUG("NextStat = %d\n",NextStat);
		FragIndex ++;
		return 0;
	}
	else if(FragIndex>1 && FragIndex <Frag_Count){

		/*	Middle packet	*/ 		
		//TX_DEBUG("send middle fragment EAP \n"); 
		pCtx->Feapol->packet_body_length = htons(EAP_HDRLEN + WSC_HAD_LEN +  pCtx->EAP_frag_threshold);
		pCtx->Feap->length= htons(EAP_HDRLEN + WSC_HAD_LEN +  pCtx->EAP_frag_threshold);
		pSta->tx_size = HEAD_LEN_a_WSC + pCtx->EAP_frag_threshold;					
		pCtx->Fwsc->flags = EAP_FR_MF ;
			
			
		memcpy(frag_buffer , pSta->tx_buffer ,HEAD_LEN_a_WSC);
		memcpy(frag_buffer+HEAD_LEN_a_WSC , copyPrt ,pCtx->EAP_frag_threshold);

		eap = (struct eap_t *)( frag_buffer + EAPOL_HDRLEN + ETHER_HDRLEN);		
		if(pCtx->is_ap){			
			pSta->eap_reqid+=1;
			if(pCtx->debug)		
				TX_DEBUG("EAP id ++ to:0x%x\n",pSta->eap_reqid);
	
		}else{
				TX_DEBUG("STA mode sync EAP-id:0x%x\n",pSta->eap_reqid);
		}
		eap->identifier = pSta->eap_reqid ;
		
		send_wlan(pCtx, frag_buffer, pSta->tx_size);
		copyPrt += pCtx->EAP_frag_threshold;
		//pSta->wait_frag_act = 1;
		pSta->state = NextStat ;
		FragIndex ++;	
		TX_DEBUG("		>>> Sending Fragment EAP (Middle)\n");
		if(pCtx->debug){
			debug_out("message data", frag_buffer+HEAD_LEN_a_WSC , pCtx->EAP_frag_threshold);
		}		

		return 0;


	}
	else if(FragIndex == Frag_Count){

		/*	Last packet	*/		
		//TX_DEBUG("send last fragment EAP \n");	
		pCtx->Feapol->packet_body_length = htons(EAP_HDRLEN + WSC_HAD_LEN +  lastMsgLen);
		pCtx->Feap->length= htons(EAP_HDRLEN + WSC_HAD_LEN +  lastMsgLen);
		pSta->tx_size = HEAD_LEN_a_WSC + lastMsgLen;					
		pCtx->Fwsc->flags = 0 ;
			
			
		memcpy(frag_buffer , pSta->tx_buffer ,HEAD_LEN_a_WSC);
		memcpy(frag_buffer+HEAD_LEN_a_WSC , copyPrt ,lastMsgLen);

		eap = (struct eap_t *)( frag_buffer + EAPOL_HDRLEN + ETHER_HDRLEN);		
		if(pCtx->is_ap){			
			pSta->eap_reqid+=1;
			if(pCtx->debug)		
				TX_DEBUG("EAP id ++ to:0x%x\n",pSta->eap_reqid);
		}else{
			TX_DEBUG("STA mode sync EAP-id:0x%x\n",pSta->eap_reqid);
		}
		eap->identifier = pSta->eap_reqid ;
		

		TX_DEBUG("		>>> Sending Fragment EAP (Last)\n");		
		if(pCtx->debug){
			debug_out("last message data",  frag_buffer+HEAD_LEN_a_WSC , lastMsgLen);
		}

		send_wlan(pCtx, frag_buffer, pSta->tx_size);

		pMsgStart = 	((unsigned char *)pCtx->Fwsc) + 10;
		memcpy(pSta->last_tx_msg_buffer, pMsgStart, pSta->sendToSta_MegTotalSize);
		pSta->last_tx_msg_size = pSta->sendToSta_MegTotalSize;			

		switch(NextStat){
			case ST_WAIT_EAPOL_FRAG_ACK_M1:
				TX_DEBUG("		>>>TX M1 completed\n");
				pSta->state = ST_WAIT_M2;				
				break;
			case ST_WAIT_EAPOL_FRAG_ACK_M2:
				if (pSta->RetVal == MSG_TYPE_M2){
					TX_DEBUG("		>>>TX M2 completed\n");	
					pSta->state = ST_WAIT_M3;
				}else{
					TX_DEBUG("		>>>TX M2D completed\n");					
					pSta->state = ST_WAIT_ACK;				
				}
				
				break;
						
			case ST_WAIT_EAPOL_FRAG_ACK_M3: 			
				TX_DEBUG("		>>>TX M3 completed\n");	
				pSta->state = ST_WAIT_M4 ;				
				break;
			case ST_WAIT_EAPOL_FRAG_ACK_M4:
				TX_DEBUG("		>>>TX M4 completed\n");	
				pSta->state = ST_WAIT_M5 ;				
				break;
				
			case ST_WAIT_EAPOL_FRAG_ACK_M5:
				TX_DEBUG("		>>>TX M5 completed\n");	
				pSta->state = ST_WAIT_M6 ;				
				break;
			case ST_WAIT_EAPOL_FRAG_ACK_M6:
				TX_DEBUG("		>>>TX M6 completed\n");	
				pSta->state = ST_WAIT_M7 ;				
				break;
				
			case ST_WAIT_EAPOL_FRAG_ACK_M7:
				TX_DEBUG("		>>>TX M7 completed\n");	
				pSta->state = ST_WAIT_M8 ;

				break;				
			case ST_WAIT_EAPOL_FRAG_ACK_M8:
				TX_DEBUG("		>>>TX M8 completed\n");	
				pSta->state = ST_WAIT_DONE ;			
				break;
			default:
				TX_DEBUG("	!!!RX unknow type %d \n",NextStat); 			
				break;				
					
		}
		
		pSta->tx_timeout = pCtx->tx_timeout;
		pSta->retry = 0;

		
		pSta->sendToSta_MegTotalSize = 0;
		

		Frag_Count = 0;		
		lastMsgLen = 0;		
		copyPrt=NULL;
		FragIndex = 0;			

		return 0 ;

	}
	else{
		TX_DEBUG("error check!!\n");
	}
	_DEBUG_PRINT("\n");							
	return 0;
}

#endif

#ifdef SUPPORT_REGISTRAR
int send_wsc_M8(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	struct eapol_t *eapol;
	struct eap_t *eap;
	struct eap_wsc_t *wsc;
	int size;
	unsigned char *pMsg, byteVal, *pMsgStart;
	unsigned char tmpbuf[1024], tmp1[1024];

	DBFENTER;
	
	build_wsc_msg_header(pCtx, pSta, WSC_OP_MSG, &eapol, &eap, &wsc);
	
	pMsg =	((unsigned char *)wsc) + sizeof(struct eap_wsc_t);
	pMsgStart = pMsg;

	byteVal = WSC_VER;
	pMsg = add_tlv(pMsg, TAG_VERSION, 1, (void *)&byteVal);
	byteVal = MSG_TYPE_M8;
	pMsg = add_tlv(pMsg, TAG_MSG_TYPE, 1, (void *)&byteVal);	
	pMsg = add_tlv(pMsg, TAG_EROLLEE_NONCE, NONCE_LEN, (void *)pSta->nonce_enrollee);

#ifdef FOR_DUAL_BAND
	if(pCtx->InterFaceComeIn==COME_FROM_WLAN0)
		size = add_credential_attr(pCtx, pSta, tmpbuf, pCtx->is_ap);
	else if(pCtx->InterFaceComeIn==COME_FROM_WLAN1)
		size = add_credential_attr2(pCtx, pSta, tmpbuf, pCtx->is_ap);
#else //FOR_DUAL_BAND
	size = add_credential_attr(pCtx, pSta, tmpbuf, pCtx->is_ap);
#endif //FOR_DUAL_BAND

	if (size < 0) {
		if (pSta->invoke_security_gen)
			pSta->invoke_security_gen = 0;
		return -1;
	}

	size = add_encrypt_attr(pSta, tmpbuf, size, tmp1);
	pMsg = add_tlv(pMsg, TAG_ENCRYPT_SETTINGS, size, (void *)tmp1);

#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2)
		pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, 6, (void *)WSC_VENDOR_V2);
#endif

	size = (int)(((unsigned long)pMsg) - ((unsigned long)pMsgStart));
	append(&pSta->last_rx_msg[pSta->last_rx_msg_size], pMsgStart, size);
	hmac_sha256(pSta->last_rx_msg, pSta->last_rx_msg_size+size, pSta->auth_key, BYTE_LEN_256B, tmp1, &size);
	pMsg = add_tlv(pMsg, TAG_AUTHENTICATOR, BYTE_LEN_64B, (void *)tmp1);

	//================================================================	
	size = (int)(((unsigned long)pMsg) - ((unsigned long)pMsgStart));

#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2) {
#ifdef EAP_FRAGMENT		
		if(pCtx->EAP_frag_threshold){
					
			if(size > pCtx->EAP_frag_threshold){
				TX_DEBUG("	>>>send  M8 first frag\n");
				pCtx->Feap = eap ;
				pCtx->Feapol = eapol ;
				pCtx->Fwsc = wsc ;	
				pSta->sendToSta_MegTotalSize = size ; 
				send_frag_msg(pCtx , pSta , ST_WAIT_EAPOL_FRAG_ACK_M8 , 1); // first time EAP Frag
				
				return 0;
			}	
		}	
#endif	
	}
#endif
	
	memcpy(pSta->last_tx_msg_buffer, pMsgStart, size);
	pSta->last_tx_msg_size = size;

	eapol->packet_body_length = htons(EAP_HDRLEN + sizeof(*wsc) + size);
	eap->length = htons(EAP_HDRLEN + sizeof(*wsc) + size);

	pSta->tx_size = ETHER_HDRLEN + EAPOL_HDRLEN + EAP_HDRLEN + sizeof(*wsc) + size;

	report_WPS_STATUS(SEND_M8);

	DEBUG_PRINT("\n>> Sending EAP WSC_MSG M8(size=%d)\n",size);

//#ifdef CONFIG_RTL8186_TR
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196C_EC)
	SET_LED_ON_FOR_10S();
#endif
		
	return send_wlan(pCtx, pSta->tx_buffer, pSta->tx_size);	
}

int send_wsc_M6(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	struct eapol_t *eapol;
	struct eap_t *eap;
	struct eap_wsc_t *wsc;
	int size;
	unsigned char *pMsg, byteVal, *pMsgStart;
	unsigned char tmpbuf[1024], tmp1[200];

	DBFENTER;
	
	build_wsc_msg_header(pCtx, pSta, WSC_OP_MSG, &eapol, &eap, &wsc);
	
	pMsg =	((unsigned char *)wsc) + sizeof(struct eap_wsc_t);
	pMsgStart = pMsg;

	byteVal = WSC_VER;
	pMsg = add_tlv(pMsg, TAG_VERSION, 1, (void *)&byteVal);
	byteVal = MSG_TYPE_M6;
	pMsg = add_tlv(pMsg, TAG_MSG_TYPE, 1, (void *)&byteVal);	
	pMsg = add_tlv(pMsg, TAG_EROLLEE_NONCE, NONCE_LEN, (void *)pSta->nonce_enrollee);

	add_tlv(tmpbuf, TAG_R_SNONCE2, NONCE_LEN, (void *)pSta->r_s2);

	size = add_encrypt_attr(pSta, tmpbuf, NONCE_LEN+4, tmp1);
	pMsg = add_tlv(pMsg, TAG_ENCRYPT_SETTINGS, size, (void *)tmp1);

#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2) {
		if(pCtx->extension_tag){
			pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, 6, (void *)WSC_VENDOR_V57);
			pMsg = add_tlv(pMsg, TAG_FOR_TEST_EXTEN, 6, (void *)EXT_ATTRI_TEST);
			
		}else{
			pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, 6, (void *)WSC_VENDOR_V2);
		}
	}
#endif

	size = (int)(((unsigned long)pMsg) - ((unsigned long)pMsgStart));
	append(&pSta->last_rx_msg[pSta->last_rx_msg_size], pMsgStart, size);
	hmac_sha256(pSta->last_rx_msg, pSta->last_rx_msg_size+size, pSta->auth_key, BYTE_LEN_256B, tmp1, &size);
	pMsg = add_tlv(pMsg, TAG_AUTHENTICATOR, BYTE_LEN_64B, (void *)tmp1);

	size = (int)(((unsigned long)pMsg) - ((unsigned long)pMsgStart));
#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2) {
#ifdef EAP_FRAGMENT		
		if(pCtx->EAP_frag_threshold){
					
			if(size > pCtx->EAP_frag_threshold){
				TX_DEBUG("	>>>send  M6 first frag\n");
				pCtx->Feap = eap ;
				pCtx->Feapol = eapol ;
				pCtx->Fwsc = wsc ;	
				pSta->sendToSta_MegTotalSize = size ; 
				//pSta->RetVal = ret ; // for identify M2 or M2D
				send_frag_msg(pCtx , pSta , ST_WAIT_EAPOL_FRAG_ACK_M6 , 1); // first time EAP Frag
				
				return 0;
			}	
		}	
#endif	
	}
#endif
	
	memcpy(pSta->last_tx_msg_buffer, pMsgStart, size);
	pSta->last_tx_msg_size = size;

	eapol->packet_body_length = htons(EAP_HDRLEN + sizeof(*wsc) + size);
	eap->length = htons(EAP_HDRLEN + sizeof(*wsc) + size);

	pSta->tx_size = ETHER_HDRLEN + EAPOL_HDRLEN + EAP_HDRLEN + sizeof(*wsc) + size;

        report_WPS_STATUS(SEND_M6);
	_DEBUG_PRINT("\n>> Sending EAP WSC_MSG M6(size=%d)\n",size);
		
	return send_wlan(pCtx, pSta->tx_buffer, pSta->tx_size);	
}

int send_wsc_M4(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	struct eapol_t *eapol;
	struct eap_t *eap;
	struct eap_wsc_t *wsc;
	int size;
	unsigned char *pMsg, byteVal, *pMsgStart;
	unsigned char tmpbuf[1024], tmp[100], tmp1[1024], tmp2[200], *ptr;

	DBFENTER;

	build_wsc_msg_header(pCtx, pSta, WSC_OP_MSG, &eapol, &eap, &wsc);

	pMsg =	((unsigned char *)wsc) + sizeof(struct eap_wsc_t);
	pMsgStart = pMsg;

	byteVal = WSC_VER;
	pMsg = add_tlv(pMsg, TAG_VERSION, 1, (void *)&byteVal);
	byteVal = MSG_TYPE_M4;
	pMsg = add_tlv(pMsg, TAG_MSG_TYPE, 1, (void *)&byteVal);
	pMsg = add_tlv(pMsg, TAG_EROLLEE_NONCE, NONCE_LEN, (void *)pSta->nonce_enrollee);

	// build R-Hash1
	hmac_sha256(pCtx->peer_pin_code, strlen(pCtx->peer_pin_code)/2, pSta->auth_key, BYTE_LEN_256B, tmp, &size);
	generate_random(pSta->r_s1, BYTE_LEN_128B);
#ifdef DEBUG
	if (pCtx->debug2)
		debug_out("R-S1",pSta->r_s1, BYTE_LEN_128B);
#endif
	memcpy(tmpbuf, pSta->r_s1, NONCE_LEN);
	ptr = append(&tmpbuf[BYTE_LEN_128B], tmp, BYTE_LEN_128B);

	BN_bn2bin(pSta->dh_enrollee->p, tmp1);
	ptr = append(ptr, tmp1, PUBLIC_KEY_LEN);

	BN_bn2bin(pSta->dh_registrar->pub_key, tmp2);
	ptr = append(ptr, tmp2, PUBLIC_KEY_LEN);

	size = (int)(((unsigned long)ptr) - ((unsigned long)tmpbuf));
	hmac_sha256(tmpbuf, size, pSta->auth_key, BYTE_LEN_256B, tmp, &size);
	pMsg = add_tlv(pMsg, TAG_R_HASH1, BYTE_LEN_256B, (void *)tmp);
#ifdef DEBUG
	if (pCtx->debug2)
		debug_out("R-Hash1", tmp, BYTE_LEN_256B);
#endif

	// build E-Hash2
	hmac_sha256(&pCtx->peer_pin_code[strlen(pCtx->peer_pin_code)/2], strlen(pCtx->peer_pin_code)/2, pSta->auth_key, BYTE_LEN_256B, tmp, &size);
	generate_random(pSta->r_s2, BYTE_LEN_128B);
#ifdef DEBUG
	if (pCtx->debug2)
		debug_out("R-S2",pSta->r_s2, BYTE_LEN_128B);
#endif
	memcpy(tmpbuf, pSta->r_s2, NONCE_LEN);
	ptr = append(&tmpbuf[BYTE_LEN_128B], tmp, BYTE_LEN_128B);
	ptr = append(ptr, tmp1, PUBLIC_KEY_LEN);
	ptr = append(ptr, tmp2, PUBLIC_KEY_LEN);

	size = (int)(((unsigned long)ptr) - ((unsigned long)tmpbuf));
	hmac_sha256(tmpbuf, size, pSta->auth_key, BYTE_LEN_256B, tmp, &size);
	pMsg = add_tlv(pMsg, TAG_R_HASH2, BYTE_LEN_256B, (void *)tmp);
#ifdef DEBUG
	if (pCtx->debug2)
		debug_out("R-Hash2", tmp, BYTE_LEN_256B);
#endif

	add_tlv(tmpbuf, TAG_R_SNONCE1, NONCE_LEN, (void *)pSta->r_s1);
	size = add_encrypt_attr(pSta, tmpbuf, NONCE_LEN+4, tmp1);
	pMsg = add_tlv(pMsg, TAG_ENCRYPT_SETTINGS, size, (void *)tmp1);
#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2) {
		if(pCtx->extension_tag){
			pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, 6, (void *)WSC_VENDOR_V57);
			pMsg = add_tlv(pMsg, TAG_FOR_TEST_EXTEN, 6, (void *)EXT_ATTRI_TEST);
			
		}else{
			pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, 6, (void *)WSC_VENDOR_V2);
		}
	}
#endif

	size = (int)(((unsigned long)pMsg) - ((unsigned long)pMsgStart));
	append(&pSta->last_rx_msg[pSta->last_rx_msg_size], pMsgStart, size);
	hmac_sha256(pSta->last_rx_msg, pSta->last_rx_msg_size+size, pSta->auth_key, BYTE_LEN_256B, tmp, &size);
	pMsg = add_tlv(pMsg, TAG_AUTHENTICATOR, BYTE_LEN_64B, (void *)tmp);


	//================================================================	
	size = (int)(((unsigned long)pMsg) - ((unsigned long)pMsgStart));
	

#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2) {
#ifdef EAP_FRAGMENT		
		if(pCtx->EAP_frag_threshold){
					
			if(size > pCtx->EAP_frag_threshold){
				TX_DEBUG("	>>>send  M4 first frag\n");
				pCtx->Feap = eap ;
				pCtx->Feapol = eapol ;
				pCtx->Fwsc = wsc ;	
				pSta->sendToSta_MegTotalSize = size ; 
				//pSta->RetVal = ret ; // for identify M2 or M2D
				send_frag_msg(pCtx , pSta , ST_WAIT_EAPOL_FRAG_ACK_M4 , 1); // first time EAP Frag
				
				return 0;
			}	
		}	
#endif	
	}
#endif

	
	//TX_DEBUG("\n");
	memcpy(pSta->last_tx_msg_buffer, pMsgStart, size);
	pSta->last_tx_msg_size = size;

	eapol->packet_body_length = htons(EAP_HDRLEN + sizeof(*wsc) + size);
	eap->length = htons(EAP_HDRLEN + sizeof(*wsc) + size);

	pSta->tx_size = ETHER_HDRLEN + EAPOL_HDRLEN + EAP_HDRLEN + sizeof(*wsc) + size;

        report_WPS_STATUS(SEND_M4);
		
	_DEBUG_PRINT("\n>> Sending EAP WSC_MSG M4(size=%d)\n",size);
	return send_wlan(pCtx, pSta->tx_buffer, pSta->tx_size);	
}

int send_wsc_M2(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	struct eapol_t *eapol;
	struct eap_t *eap;
	struct eap_wsc_t *wsc;
	int size, id=-1;
	unsigned char *pMsg, byteVal, *pMsgStart;
	unsigned char tmpbuf[200];
	unsigned short shortVal;
	unsigned long longVal;
	DH *a=NULL;
	int ret;

	DBFENTER;

	report_WPS_STATUS(SEND_M2);

#ifdef WPS2DOT0_DEBUG
	TX_DEBUG("STA's dev_password_id=0x%x\n",pSta->device_password_id);
	MAC_PRINT(pSta->addr);
#endif

	build_wsc_msg_header(pCtx, pSta, WSC_OP_MSG, &eapol, &eap, &wsc);

	pMsg =	((unsigned char *)wsc) + sizeof(struct eap_wsc_t);
	pMsgStart = pMsg;

	byteVal = WSC_VER;
	pMsg = add_tlv(pMsg, TAG_VERSION, 1, (void *)&byteVal);	

	// for interoperability in Japan
	if (pSta->device_password_id == PASS_ID_DEFAULT ||
		pSta->device_password_id == PASS_ID_USER ||
		pSta->device_password_id == PASS_ID_MACHINE ||
		pSta->device_password_id == PASS_ID_REKEY ||
		pSta->device_password_id == PASS_ID_REG) {

		if (IS_PIN_METHOD(pCtx->config_method) && pCtx->pin_assigned)	
		{
			if (pCtx->peer_pin_id == PASS_ID_USER){
				id = PASS_ID_USER;
				TX_DEBUG("pass id assigned to 0x%x\n",id);
			}else{
				id = PASS_ID_DEFAULT;	
				TX_DEBUG("pass id assigned to 0x%x\n",id);				
			}
		}
	}
	else if (pSta->device_password_id == PASS_ID_PB) {	
		if (IS_PBC_METHOD(pCtx->config_method) && pCtx->pb_pressed) {
#ifdef MUL_PBC_DETECTTION
			if (pCtx->is_ap) {
				if (!pCtx->disable_MulPBC_detection) {
				WSC_pthread_mutex_lock(&pCtx->PBCMutex);
				//DEBUG_PRINT("%s %d Lock PBC mutex\n", __FUNCTION__, __LINE__);
				
				search_active_pbc_sta(pCtx, pSta->addr, pSta->uuid);
				if (pCtx->active_pbc_sta_count > 1) {
						TX_DEBUG("		!!!Multiple PBC sessions [%d] detected!\n\n\n", pCtx->active_pbc_sta_count);
					WSC_pthread_mutex_unlock(&pCtx->PBCMutex);
					//DEBUG_PRINT("%s %d unlock PBC mutex\n", __FUNCTION__, __LINE__);
					
					SwitchSessionOverlap_LED_On(pCtx);
					id = -2;
				}
				else {
					WSC_pthread_mutex_unlock(&pCtx->PBCMutex);
					//DEBUG_PRINT("%s %d unlock PBC mutex\n", __FUNCTION__, __LINE__);
					id = PASS_ID_PB;
				}
			}
			else
					id = PASS_ID_PB;
			}
			else
#endif
				id = PASS_ID_PB;
		}
	}

	if (id < 0)
		byteVal = MSG_TYPE_M2D;
	else	 {
		WSC_pthread_mutex_lock(&pCtx->RegMutex);
		//DEBUG_PRINT("%s %d Lock mutex\n", __FUNCTION__, __LINE__);
		if (pCtx->registration_on >= 1 && pCtx->sta_invoke_reg != pSta) {
			DEBUG_PRINT("%s %d Registration protocol is already in progress; abort send_wsc_M2 \n", __FUNCTION__, __LINE__);
			// Reason code 5 : Disassociated because AP is unable to handle all currently associated stations
			if (pCtx->is_ap){
				IssueDisconnect(pCtx->wlan_interface_name, pSta->addr, 5);
				TX_DEBUG("IssueDisconnect\n\n");
			}
			if (!(pSta->used & IS_UPNP_CONTROL_POINT))
				reset_sta(pCtx, pSta, 1);
			else
				reset_sta(pCtx, pSta, 0);

			WSC_pthread_mutex_unlock(&pCtx->RegMutex);
			//DEBUG_PRINT("%s %d unlock mutex\n", __FUNCTION__, __LINE__);

			return -1;
		}
		else {
			pCtx->registration_on = 1;

			DEBUG_PRINT2("Set Registration_on to %d\n", pCtx->registration_on);
			
			pCtx->sta_invoke_reg = pSta;

			if (pCtx->pb_pressed) {
				strcpy(pCtx->peer_pin_code, "00000000");
				pCtx->pin_assigned = 1;
				DEBUG_PRINT("%s %d set pCtx->peer_pin_code = 00000000 due to PBC\n", __FUNCTION__, __LINE__);
			}
			WSC_pthread_mutex_unlock(&pCtx->RegMutex);
			//DEBUG_PRINT("%s %d unlock mutex\n", __FUNCTION__, __LINE__);
		}
		byteVal = MSG_TYPE_M2;	
	}
	ret = (int)byteVal;
	pMsg = add_tlv(pMsg, TAG_MSG_TYPE, 1, (void *)&byteVal);
	pMsg = add_tlv(pMsg, TAG_EROLLEE_NONCE, NONCE_LEN, (void *)pSta->nonce_enrollee);
	generate_random(tmpbuf, NONCE_LEN);
	pMsg = add_tlv(pMsg, TAG_REGISTRAR_NONCE, NONCE_LEN, (void *)tmpbuf);
	memcpy(pSta->nonce_registrar, tmpbuf, NONCE_LEN);
	pMsg = add_tlv(pMsg, TAG_UUID_R, UUID_LEN, (void *)pCtx->uuid);

	if (id >= 0) {
		a = generate_dh_parameters(PUBLIC_KEY_LEN*8, wsc_prime_num, DH_GENERATOR_2);
		if (a == NULL)
			return -1;		
		a->flags &= ~DH_FLAG_NO_EXP_CONSTTIME;	
		if (!DH_generate_key(a)) {
			if (a){
				DH_free(a);
				a = NULL;
			}			
			DEBUG_ERR("DH_generate_key(a) error!\n");
			return -1;		
		}

#ifdef DEBUG
		if (pCtx->debug2) {
		//	printf("pri 1=");
		//	bn_printxxxx(a->priv_key);
			_DEBUG_PRINT("\nDH public-key=");
			bn_printxxxx(a->pub_key);
			_DEBUG_PRINT("\n");	
		}
#endif	

		if (BN_bn2bin(a->pub_key, tmpbuf) != PUBLIC_KEY_LEN) {
			TX_DEBUG("Invalid public key length [%d]\n", size);
			if (a){
				DH_free(a);
				a = NULL;
			}
			return -1;
		}
		pMsg = add_tlv(pMsg, TAG_PUB_KEY, PUBLIC_KEY_LEN, (void *)tmpbuf);
		pSta->dh_registrar = a;
	}		

	shortVal = htons(((unsigned short)pCtx->auth_type_flags));
	pMsg = add_tlv(pMsg, TAG_AUTH_TYPE_FLAGS, 2, (void *)&shortVal);
	shortVal = htons(((unsigned short)pCtx->encrypt_type_flags));
	pMsg = add_tlv(pMsg, TAG_ENCRYPT_TYPE_FLAGS, 2, (void *)&shortVal);
	byteVal = (unsigned char)pCtx->connect_type;
	pMsg = add_tlv(pMsg, TAG_CONNECT_TYPE_FLAGS, 1, (void *)&byteVal);
	shortVal = htons(((unsigned short)pCtx->config_method));
	pMsg = add_tlv(pMsg, TAG_CONFIG_METHODS, 2, (void *)&shortVal);
	byteVal = (unsigned char)pCtx->config_state;	

	size = strlen(pCtx->manufacturer);
	pMsg = add_tlv(pMsg, TAG_MANUFACTURER, size, (void *)pCtx->manufacturer);

	size = strlen(pCtx->model_name);	
	pMsg = add_tlv(pMsg, TAG_MODEL_NAME, size, (void *)pCtx->model_name);		

	size = strlen(pCtx->model_num);
	pMsg = add_tlv(pMsg, TAG_MODEL_NUMBER, size, (void *)pCtx->model_num);

	size = strlen(pCtx->serial_num);	
	pMsg = add_tlv(pMsg, TAG_SERIAL_NUM, size, (void *)pCtx->serial_num);
	
	shortVal = htons(((unsigned short)pCtx->device_category_id));
	memcpy(tmpbuf, &shortVal, 2);
	memcpy(&tmpbuf[2], pCtx->device_oui, OUI_LEN);
	shortVal = htons(((unsigned short)pCtx->device_sub_category_id));
	memcpy(&tmpbuf[6], &shortVal, 2);	
	pMsg = add_tlv(pMsg, TAG_PRIMARY_DEVICE_TYPE, 8, (void *)tmpbuf);



	pMsg = add_tlv(pMsg, TAG_DEVICE_NAME, strlen(pCtx->device_name), (void *)pCtx->device_name);	

#ifdef FOR_DUAL_BAND
	if(pCtx->is_ap){
		if(pCtx->InterFaceComeIn==COME_FROM_WLAN0)
			// use rf_band from wlan0's flash mib ;because wlan0 maybe 5g or 2.4g
			byteVal = (unsigned char)pCtx->rf_band;	
		else if(pCtx->InterFaceComeIn==COME_FROM_WLAN1)
			byteVal =1;
		else
			byteVal =1; // default use 2.4g
	}
	else{
		if(pCtx->STAmodeNegoWith == COMEFROM5G){
			byteVal =2;
		}
		else if(pCtx->STAmodeNegoWith == COMEFROM24G){
			byteVal =1;
		}
		else
		{
			byteVal =1; // default use 2.4g
		}
	}
#else //FOR_DUAL_BAND
	byteVal = (unsigned char)pCtx->rf_band;
#endif //FOR_DUAL_BAND
	pMsg = add_tlv(pMsg, TAG_RF_BAND, 1, (void *)&byteVal);
	shortVal = ASSOC_STATE_CONNECT_SUCCESS; // TODO:
	shortVal = htons((unsigned short)shortVal);
	pMsg = add_tlv(pMsg, TAG_ASSOC_STATE, 2, (void *)&shortVal);	

#ifdef MUL_PBC_DETECTTION
	if (id == -2 && !pCtx->disable_MulPBC_detection)
		shortVal = htons(CONFIG_ERR_MUL_PBC_DETECTED);
	else
#endif
		shortVal = htons((unsigned short)pCtx->config_err);
	pMsg = add_tlv(pMsg, TAG_CONFIG_ERR, 2, (void *)&shortVal);
			
	if (id >= 0) {
		shortVal = htons((unsigned short)id);
		pMsg = add_tlv(pMsg, TAG_DEVICE_PASSWORD_ID, 2, (void *)&shortVal);
	}
	
	longVal = htonl(pCtx->os_ver | 0x10000000);	
	pMsg = add_tlv(pMsg, TAG_OS_VERSION, 4, (void *)&longVal);

#ifdef WPS2DOTX	// for plugfest need , normal case no need it
	if (pCtx->current_wps_version == WPS_VERSION_V2) {
		if(pCtx->extension_tag){
			pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, 6, (void *)WSC_VENDOR_V57);
			pMsg = add_tlv(pMsg, TAG_FOR_TEST_EXTEN, 6, (void *)EXT_ATTRI_TEST);
			
		}else{
			pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, 6, (void *)WSC_VENDOR_V2);
		}
	}
#endif


	if (id >= 0) {
		if (derive_key(pCtx, pSta) < 0) {
			if (a){
				DH_free(a);
				a = NULL;
			}
			return -1;
		}
		size = (int)(((unsigned long)pMsg) - ((unsigned long)pMsgStart));
		append(&pSta->last_rx_msg[pSta->last_rx_msg_size], pMsgStart, size);

		hmac_sha256(pSta->last_rx_msg, pSta->last_rx_msg_size+size, 
			pSta->auth_key, BYTE_LEN_256B, tmpbuf, &size);
		
		pMsg = add_tlv(pMsg, TAG_AUTHENTICATOR, BYTE_LEN_64B, (void *)tmpbuf);
	}
	
	size = (int)(((unsigned long)pMsg) - ((unsigned long)pMsgStart));

#ifdef WPS2DOTX
#ifdef EAP_FRAGMENT
	if (pCtx->current_wps_version == WPS_VERSION_V2) {
		if(pCtx->EAP_frag_threshold){
					
			if(size > pCtx->EAP_frag_threshold){
				TX_DEBUG("send  M2 message Frag1...\n");
				pCtx->Feap = eap ;
				pCtx->Feapol = eapol ;
				pCtx->Fwsc = wsc ;	
				pSta->sendToSta_MegTotalSize = size ; 
				pSta->RetVal = ret ; // for identify M2 or M2D
				send_frag_msg(pCtx , pSta , ST_WAIT_EAPOL_FRAG_ACK_M2 , 1); // first time EAP Frag

				return 0;
			}	
		}	
	}
#endif	
#endif


	memcpy(&pSta->last_tx_msg_buffer, pMsgStart, size);
	pSta->last_tx_msg_size = size;

	eapol->packet_body_length = htons(EAP_HDRLEN + sizeof(*wsc) + size);
	eap->length = htons(EAP_HDRLEN + sizeof(*wsc) + size);

	pSta->tx_size = ETHER_HDRLEN + EAPOL_HDRLEN + EAP_HDRLEN + sizeof(*wsc) + size;

	report_WPS_STATUS(SEND_M2);		

	if (id < 0) {
		DEBUG_PRINT(">> Sending EAP WSC_MSG M2D(size=%d)\n",size);
	}
	else {
		DEBUG_PRINT(">> Sending EAP WSC_MSG M2(size=%d)\n",size);
	}
		
	if (send_wlan(pCtx, pSta->tx_buffer, pSta->tx_size) < 0) {
		if (a){
			DH_free(a);
			a = NULL;
		}
		return -1;
	}
	else
		return ret;
}
#endif // SUPPORT_REGISTRAR

#ifdef SUPPORT_ENROLLEE
int send_wsc_M7(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	struct eapol_t *eapol;
	struct eap_t *eap;
	struct eap_wsc_t *wsc;
	int size;
	unsigned char *pMsg, byteVal, *pMsgStart, *p;
	unsigned char tmpbuf[1024], tmp1[512];
	unsigned short shortVal;

	unsigned short AuthType = 0;
	unsigned short EncryType = 0;

	DBFENTER;

	report_WPS_STATUS(SEND_M7);

#ifdef SUPPORT_UPNP
	if (pSta->used & IS_UPNP_CONTROL_POINT) 
		pMsg = pSta->tx_buffer;
	else
#endif		
	{
		build_wsc_msg_header(pCtx, pSta, WSC_OP_MSG, &eapol, &eap, &wsc);	
		pMsg =	((unsigned char *)wsc) + sizeof(struct eap_wsc_t);
	}
	pMsgStart = pMsg;

	byteVal = WSC_VER;
	pMsg = add_tlv(pMsg, TAG_VERSION, 1, (void *)&byteVal);
	byteVal = MSG_TYPE_M7;
	pMsg = add_tlv(pMsg, TAG_MSG_TYPE, 1, (void *)&byteVal);	
	pMsg = add_tlv(pMsg, TAG_REGISTRAR_NONCE, NONCE_LEN, (void *)pSta->nonce_registrar);
	p = add_tlv(tmpbuf, TAG_E_SNONCE2, NONCE_LEN, (void *)pSta->e_s2);
	
	if (pCtx->is_ap) {
#ifdef FOR_DUAL_BAND		
		if(pCtx->InterFaceComeIn == COME_FROM_WLAN1){
			p = add_tlv(p, TAG_SSID, strlen(pCtx->SSID2), pCtx->SSID2); 	
			p = add_tlv(p, TAG_MAC_ADDRESS, ETHER_ADDRLEN, pCtx->our_addr2);
			
			AuthType = pCtx->auth_type2;
			EncryType = pCtx->encrypt_type2;
			
			if (pCtx->mixedmode2) {
				if(((pSta->auth_type_flags & AUTH_WPA2PSK) && (pSta->encrypt_type_flags & ENCRYPT_AES))
					&& (pCtx->mixedmode2 & WSC_WPA2_AES))
				{
					AuthType=AUTH_WPA2PSK;
					EncryType=ENCRYPT_AES;
				}else if(((pSta->auth_type_flags & AUTH_WPAPSK) && (pSta->encrypt_type_flags & ENCRYPT_TKIP))
					&& (pCtx->mixedmode2 & WSC_WPA_TKIP)){
					AuthType=AUTH_WPAPSK;
					EncryType=ENCRYPT_TKIP;
				}
				else if(((pSta->auth_type_flags & AUTH_WPAPSK) && (pSta->encrypt_type_flags & ENCRYPT_AES))
					&& (pCtx->mixedmode2 & WSC_WPA_AES)){
					AuthType=AUTH_WPAPSK;
					EncryType=ENCRYPT_AES;
				}
				else if(((pSta->auth_type_flags & AUTH_WPA2PSK) && (pSta->encrypt_type_flags & ENCRYPT_TKIP))
					&& (pCtx->mixedmode2 & WSC_WPA2_TKIP)){
					AuthType=AUTH_WPA2PSK;
					EncryType=ENCRYPT_TKIP;
				}else{
					TX_DEBUG("can't support STA's security capability\n");	
				}				
			}
			shortVal = htons(AuthType);
			#ifdef WPS2DOTX
			if (((AuthType&( AUTH_OPEN | AUTH_WPA2 | AUTH_WPA2PSK | AUTH_WPA2PSKMIXED ))==0) && (pCtx->current_wps_version == WPS_VERSION_V2))
			{
				TX_DEBUG("AP uses auth_type= 0x%x; not supported by 2.0\n", AuthType);
					return -1;
			}	
			#endif
			p = add_tlv(p, TAG_AUTH_TYPE, 2, &shortVal);
			
			shortVal = htons(EncryType);
			#ifdef WPS2DOTX
			if ((EncryType==ENCRYPT_WEP) && (pCtx->current_wps_version == WPS_VERSION_V2)){
				TX_DEBUG("WEP Encrpty type not support under WPS2.0\n");
			}
			#endif		
			p = add_tlv(p, TAG_ENCRYPT_TYPE, 2, &shortVal);
			
			byteVal = 1;
			p = add_tlv(p, TAG_NETWORK_KEY_INDEX, 1, &byteVal); 	
			p = add_tlv(p, TAG_NETWORK_KEY, pCtx->network_key_len2, pCtx->network_key2);
			if (pCtx->encrypt_type2 == ENCRYPT_WEP) {
				byteVal = 2;
				p = add_tlv(p, TAG_NETWORK_KEY_INDEX, 1, &byteVal); 	
				p = add_tlv(p, TAG_NETWORK_KEY, strlen(pCtx->wep_key22), pCtx->wep_key22);
				byteVal = 3;
				p = add_tlv(p, TAG_NETWORK_KEY_INDEX, 1, &byteVal); 				
				p = add_tlv(p, TAG_NETWORK_KEY, strlen(pCtx->wep_key32), pCtx->wep_key32);
				byteVal = 4;
				p = add_tlv(p, TAG_NETWORK_KEY_INDEX, 1, &byteVal); 		
				p = add_tlv(p, TAG_NETWORK_KEY, strlen(pCtx->wep_key42), pCtx->wep_key42);
				p = add_tlv(p, TAG_WEP_TRANSMIT_KEY, 1, &pCtx->wep_transmit_key2);
			}		
			
		}
		else{
			if(pCtx->InterFaceComeIn != COME_FROM_WLAN0) {
				TX_DEBUG("unknown case chk!!!\n\n");
			}
			p = add_tlv(p, TAG_SSID, strlen(pCtx->SSID), pCtx->SSID);		
			p = add_tlv(p, TAG_MAC_ADDRESS, ETHER_ADDRLEN, pCtx->our_addr);

			AuthType = pCtx->auth_type;
			EncryType = pCtx->encrypt_type;
			
			if (pCtx->mixedmode) {
				if(((pSta->auth_type_flags & AUTH_WPA2PSK) && (pSta->encrypt_type_flags & ENCRYPT_AES))
					&& (pCtx->mixedmode & WSC_WPA2_AES))
				{
					AuthType=AUTH_WPA2PSK;
					EncryType=ENCRYPT_AES;
				}else if(((pSta->auth_type_flags & AUTH_WPAPSK) && (pSta->encrypt_type_flags & ENCRYPT_TKIP))
					&& (pCtx->mixedmode & WSC_WPA_TKIP)){
					AuthType=AUTH_WPAPSK;
					EncryType=ENCRYPT_TKIP;
				}
				else if(((pSta->auth_type_flags & AUTH_WPAPSK) && (pSta->encrypt_type_flags & ENCRYPT_AES))
					&& (pCtx->mixedmode & WSC_WPA_AES)){
					AuthType=AUTH_WPAPSK;
					EncryType=ENCRYPT_AES;
				}
				else if(((pSta->auth_type_flags & AUTH_WPA2PSK) && (pSta->encrypt_type_flags & ENCRYPT_TKIP))
					&& (pCtx->mixedmode & WSC_WPA2_TKIP)){
					AuthType=AUTH_WPA2PSK;
					EncryType=ENCRYPT_TKIP;
				}else{
					TX_DEBUG("can't support STA's security capability\n");
				}
			}
			
			shortVal = htons(AuthType);
			#ifdef WPS2DOTX
			if (((AuthType&( AUTH_OPEN | AUTH_WPA2 | AUTH_WPA2PSK | AUTH_WPA2PSKMIXED ))==0) && (pCtx->current_wps_version == WPS_VERSION_V2))
			{
				TX_DEBUG("AP uses auth_type= 0x%x; not supported by 2.0\n", AuthType);
				return -1;
			}	
			#endif
			p = add_tlv(p, TAG_AUTH_TYPE, 2, &shortVal);
			
			shortVal = htons(EncryType);
			#ifdef WPS2DOTX
			if ((EncryType==ENCRYPT_WEP) && (pCtx->current_wps_version == WPS_VERSION_V2)){
				TX_DEBUG("WEP Encrpty type not support under WPS2.0\n");
			}
			#endif		
			p = add_tlv(p, TAG_ENCRYPT_TYPE, 2, &shortVal);
			
			byteVal = 1;
			p = add_tlv(p, TAG_NETWORK_KEY_INDEX, 1, &byteVal); 	
			p = add_tlv(p, TAG_NETWORK_KEY, pCtx->network_key_len, pCtx->network_key);
			if (pCtx->encrypt_type == ENCRYPT_WEP) {
				byteVal = 2;
				p = add_tlv(p, TAG_NETWORK_KEY_INDEX, 1, &byteVal); 	
				p = add_tlv(p, TAG_NETWORK_KEY, strlen(pCtx->wep_key2), pCtx->wep_key2);
				byteVal = 3;
				p = add_tlv(p, TAG_NETWORK_KEY_INDEX, 1, &byteVal); 				
				p = add_tlv(p, TAG_NETWORK_KEY, strlen(pCtx->wep_key3), pCtx->wep_key3);
				byteVal = 4;
				p = add_tlv(p, TAG_NETWORK_KEY_INDEX, 1, &byteVal); 		
				p = add_tlv(p, TAG_NETWORK_KEY, strlen(pCtx->wep_key4), pCtx->wep_key4);
				p = add_tlv(p, TAG_WEP_TRANSMIT_KEY, 1, &pCtx->wep_transmit_key);
			}		
			
		}
#else //FOR_DUAL_BAND
		p = add_tlv(p, TAG_SSID, strlen(pCtx->SSID), pCtx->SSID);
		p = add_tlv(p, TAG_MAC_ADDRESS, ETHER_ADDRLEN, pCtx->our_addr);

		AuthType = pCtx->auth_type;
		EncryType = pCtx->encrypt_type;
		
		if (pCtx->mixedmode) {
			if(((pSta->auth_type_flags & AUTH_WPA2PSK) && (pSta->encrypt_type_flags & ENCRYPT_AES))
				&& (pCtx->mixedmode & WSC_WPA2_AES))
			{
				AuthType=AUTH_WPA2PSK;
				EncryType=ENCRYPT_AES;
			}else if(((pSta->auth_type_flags & AUTH_WPAPSK) && (pSta->encrypt_type_flags & ENCRYPT_TKIP))
				&& (pCtx->mixedmode & WSC_WPA_TKIP)){
				AuthType=AUTH_WPAPSK;
				EncryType=ENCRYPT_TKIP;
			}
			else if(((pSta->auth_type_flags & AUTH_WPAPSK) && (pSta->encrypt_type_flags & ENCRYPT_AES))
				&& (pCtx->mixedmode & WSC_WPA_AES)){
				AuthType=AUTH_WPAPSK;
				EncryType=ENCRYPT_AES;
			}
			else if(((pSta->auth_type_flags & AUTH_WPA2PSK) && (pSta->encrypt_type_flags & ENCRYPT_TKIP))
				&& (pCtx->mixedmode & WSC_WPA2_TKIP)){
				AuthType=AUTH_WPA2PSK;
				EncryType=ENCRYPT_TKIP;
			}else{
				TX_DEBUG("can't support STA's security capability\n");
			}
		}

		shortVal = htons(AuthType);
		p = add_tlv(p, TAG_AUTH_TYPE, 2, &shortVal);
		
		shortVal = htons(EncryType);
		#ifdef WPS2DOTX
		if((EncryType==ENCRYPT_WEP) && (pCtx->current_wps_version == WPS_VERSION_V2)){
			TX_DEBUG("WEP Encrpty type not support under WPS2.0\n");
		}
		#endif		
		p = add_tlv(p, TAG_ENCRYPT_TYPE, 2, &shortVal);
		
		byteVal = 1;
		p = add_tlv(p, TAG_NETWORK_KEY_INDEX, 1, &byteVal);		
		p = add_tlv(p, TAG_NETWORK_KEY, pCtx->network_key_len, pCtx->network_key);
		if (pCtx->encrypt_type == ENCRYPT_WEP) {
			byteVal = 2;
			p = add_tlv(p, TAG_NETWORK_KEY_INDEX, 1, &byteVal);		
			p = add_tlv(p, TAG_NETWORK_KEY, strlen(pCtx->wep_key2), pCtx->wep_key2);
			byteVal = 3;
			p = add_tlv(p, TAG_NETWORK_KEY_INDEX, 1, &byteVal);					
			p = add_tlv(p, TAG_NETWORK_KEY, strlen(pCtx->wep_key3), pCtx->wep_key3);
			byteVal = 4;
			p = add_tlv(p, TAG_NETWORK_KEY_INDEX, 1, &byteVal);			
			p = add_tlv(p, TAG_NETWORK_KEY, strlen(pCtx->wep_key4), pCtx->wep_key4);
			p = add_tlv(p, TAG_WEP_TRANSMIT_KEY, 1, &pCtx->wep_transmit_key);
		}
#endif //FOR_DUAL_BAND
	}

	size = (int)(((unsigned long)p) - ((unsigned long)tmpbuf));
	size = add_encrypt_attr(pSta, tmpbuf, size, tmp1);
	pMsg = add_tlv(pMsg, TAG_ENCRYPT_SETTINGS, size, (void *)tmp1);

#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2)
		pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, 6, (void *)WSC_VENDOR_V2);
#endif


	size = (int)(((unsigned long)pMsg) - ((unsigned long)pMsgStart));
	append(&pSta->last_rx_msg[pSta->last_rx_msg_size], pMsgStart, size);
	hmac_sha256(pSta->last_rx_msg, pSta->last_rx_msg_size+size, pSta->auth_key, BYTE_LEN_256B, tmp1, &size);
	pMsg = add_tlv(pMsg, TAG_AUTHENTICATOR, BYTE_LEN_64B, (void *)tmp1);


	//================================================================	

	size = (int)(((unsigned long)pMsg) - ((unsigned long)pMsgStart));
#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2) {
#ifdef EAP_FRAGMENT
#ifdef SUPPORT_UPNP
		/*cond2,we don't frag when AP as enrollee,(by ER config) ; WPS2DOTX */ 
		if (!(pSta->used & IS_UPNP_CONTROL_POINT)  &&  (!pCtx->is_ap))  
#endif		
		{
			if(pCtx->EAP_frag_threshold){
				
				if(size > pCtx->EAP_frag_threshold){
					//pCtx->origeMgsSize=size;
					pCtx->Feap = eap ;
					pCtx->Feapol = eapol ;
					pCtx->Fwsc = wsc ;
					pSta->sendToSta_MegTotalSize = size ; 
					send_frag_msg(pCtx , pSta ,ST_WAIT_EAPOL_FRAG_ACK_M7,1 ); // first time EAP Frag
					return 0;
				}	
			}
		}
#endif	
	}
#endif
	
	memcpy(pSta->last_tx_msg_buffer, pMsgStart, size);
	pSta->last_tx_msg_size = size;

#ifdef SUPPORT_UPNP
	if (!(pSta->used & IS_UPNP_CONTROL_POINT)) 
#endif	
	{
		eapol->packet_body_length = htons(EAP_HDRLEN + sizeof(*wsc) + size);
		eap->length = htons(EAP_HDRLEN + sizeof(*wsc) + size);
		pSta->tx_size = ETHER_HDRLEN + EAPOL_HDRLEN + EAP_HDRLEN + sizeof(*wsc) + size;
	}

	_DEBUG_PRINT("\n>> Sending EAP WSC_MSG M7\n");

#ifdef SUPPORT_UPNP
	if (pSta->used & IS_UPNP_CONTROL_POINT)
		return 0;
	else
#endif		
		return send_wlan(pCtx, pSta->tx_buffer, pSta->tx_size);
}

int send_wsc_M5(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	struct eapol_t *eapol;
	struct eap_t *eap;
	struct eap_wsc_t *wsc;
	int size;
	unsigned char *pMsg, byteVal, *pMsgStart;
	unsigned char tmpbuf[1024], tmp1[200];

	DBFENTER;

#ifdef SUPPORT_UPNP
	if (pSta->used & IS_UPNP_CONTROL_POINT) 
		pMsg = pSta->tx_buffer;
	else
#endif		
	{
		build_wsc_msg_header(pCtx, pSta, WSC_OP_MSG, &eapol, &eap, &wsc);	
		pMsg =	((unsigned char *)wsc) + sizeof(struct eap_wsc_t);
	}
	pMsgStart = pMsg;

	byteVal = WSC_VER;
	pMsg = add_tlv(pMsg, TAG_VERSION, 1, (void *)&byteVal);
	byteVal = MSG_TYPE_M5;
	pMsg = add_tlv(pMsg, TAG_MSG_TYPE, 1, (void *)&byteVal);	
	pMsg = add_tlv(pMsg, TAG_REGISTRAR_NONCE, NONCE_LEN, (void *)pSta->nonce_registrar);

	add_tlv(tmpbuf, TAG_E_SNONCE1, NONCE_LEN, (void *)pSta->e_s1);

	size = add_encrypt_attr(pSta, tmpbuf, NONCE_LEN+4, tmp1);
	pMsg = add_tlv(pMsg, TAG_ENCRYPT_SETTINGS, size, (void *)tmp1);

#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2)
		pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, 6, (void *)WSC_VENDOR_V2);
#endif



	size = (int)(((unsigned long)pMsg) - ((unsigned long)pMsgStart));
	append(&pSta->last_rx_msg[pSta->last_rx_msg_size], pMsgStart, size);
	hmac_sha256(pSta->last_rx_msg, pSta->last_rx_msg_size+size, pSta->auth_key, BYTE_LEN_256B, tmp1, &size);
	pMsg = add_tlv(pMsg, TAG_AUTHENTICATOR, BYTE_LEN_64B, (void *)tmp1);


	//================================================================	

	size = (int)(((unsigned long)pMsg) - ((unsigned long)pMsgStart));

#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2) {
#ifdef EAP_FRAGMENT
#ifdef SUPPORT_UPNP
		/*cond2,we don't frag when AP as enrollee,(by ER config) ; WPS2DOTX */ 
		if (!(pSta->used & IS_UPNP_CONTROL_POINT)  &&  (!pCtx->is_ap))  
#endif		
		{
			if(pCtx->EAP_frag_threshold){
				
				if(size > pCtx->EAP_frag_threshold){
					//pCtx->origeMgsSize=size;
					pCtx->Feap = eap ;
					pCtx->Feapol = eapol ;
					pCtx->Fwsc = wsc ;
					pSta->sendToSta_MegTotalSize = size ; 
					send_frag_msg(pCtx , pSta ,ST_WAIT_EAPOL_FRAG_ACK_M5,1 ); // first time EAP Frag
					return 0;
				}	
			}
		}
#endif	
	}
#endif

	
	memcpy(pSta->last_tx_msg_buffer, pMsgStart, size);
	pSta->last_tx_msg_size = size;

#ifdef SUPPORT_UPNP
	if (!(pSta->used & IS_UPNP_CONTROL_POINT)) 
#endif	
	{
		eapol->packet_body_length = htons(EAP_HDRLEN + sizeof(*wsc) + size);
		eap->length = htons(EAP_HDRLEN + sizeof(*wsc) + size);
		pSta->tx_size = ETHER_HDRLEN + EAPOL_HDRLEN + EAP_HDRLEN + sizeof(*wsc) + size;
	}

	report_WPS_STATUS(SEND_M5);
	DEBUG_PRINT("\n>> Sending EAP WSC_MSG M5\n");

#ifdef SUPPORT_UPNP
	if (pSta->used & IS_UPNP_CONTROL_POINT)
		return 0;
	else
#endif		
		return send_wlan(pCtx, pSta->tx_buffer, pSta->tx_size);
}

int send_wsc_M3(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	struct eapol_t *eapol;
	struct eap_t *eap;
	struct eap_wsc_t *wsc;
	int size;
	unsigned char *pMsg, byteVal, *pMsgStart;
	unsigned char tmpbuf[1024], tmp[100], tmp1[200], tmp2[200], *ptr;
    int idx=0;

                  

	DBFENTER;
#ifdef SUPPORT_UPNP
	if (pSta->used & IS_UPNP_CONTROL_POINT) 
		pMsg = pSta->tx_buffer;
	else
#endif		
	{
		build_wsc_msg_header(pCtx, pSta, WSC_OP_MSG, &eapol, &eap, &wsc);
		pMsg =	((unsigned char *)wsc) + sizeof(struct eap_wsc_t);
	}
	pMsgStart = pMsg;

	byteVal = WSC_VER;
	pMsg = add_tlv(pMsg, TAG_VERSION, 1, (void *)&byteVal);
	byteVal = MSG_TYPE_M3;
	pMsg = add_tlv(pMsg, TAG_MSG_TYPE, 1, (void *)&byteVal);
	pMsg = add_tlv(pMsg, TAG_REGISTRAR_NONCE, NONCE_LEN, (void *)pSta->nonce_registrar);

	// build E-Hash1
	hmac_sha256((unsigned char*)pCtx->pin_code, strlen(pCtx->pin_code)/2, pSta->auth_key, BYTE_LEN_256B, tmp, &size);
	generate_random(pSta->e_s1, BYTE_LEN_128B); // 16Bytes
	#ifdef DEBUG_PIXIE_DUST_ATTACK
	printf("\n E_S1 \n");
    for(idx=0;idx<NONCE_LEN;idx++)
        printf("%02x  ",pSta->e_s1[idx]);
    printf("\n");
	printf("E_S1 end\n\n");	
	#endif	
#ifdef DEBUG
	if (pCtx->debug2)
		debug_out("E-S1",pSta->e_s1, BYTE_LEN_128B);
#endif
	memcpy(tmpbuf, pSta->e_s1, NONCE_LEN);
	ptr = append(&tmpbuf[BYTE_LEN_128B], tmp, BYTE_LEN_128B);

	BN_bn2bin(pSta->dh_enrollee->pub_key, tmp1);
	ptr = append(ptr, tmp1, PUBLIC_KEY_LEN);

	BN_bn2bin(pSta->dh_registrar->p, tmp2);
	ptr = append(ptr, tmp2, PUBLIC_KEY_LEN);

	size = (int)(((unsigned long)ptr) - ((unsigned long)tmpbuf));
	hmac_sha256(tmpbuf, size, pSta->auth_key, BYTE_LEN_256B, tmp, &size);
	pMsg = add_tlv(pMsg, TAG_E_HASH1, BYTE_LEN_256B, (void *)tmp);
#ifdef DEBUG
	if (pCtx->debug2)
		debug_out("E-Hash1", tmp, BYTE_LEN_256B);
#endif

	// build E-Hash2
	hmac_sha256(&pCtx->pin_code[strlen(pCtx->pin_code)/2], strlen(pCtx->pin_code)/2, pSta->auth_key, BYTE_LEN_256B, tmp, &size);

	generate_random(pSta->e_s2, BYTE_LEN_128B);
	memcpy(tmpbuf, pSta->e_s2, NONCE_LEN);

	#ifdef DEBUG_PIXIE_DUST_ATTACK
	printf("\nE_S2\n");	
    for(idx=0;idx<NONCE_LEN;idx++)
        printf("%02x  ",pSta->e_s2[idx]);
    printf("\n"); 
	printf("\nE_S2 end\n");	    
	#endif
#ifdef DEBUG
	if (pCtx->debug2)
		debug_out("E-S2",pSta->e_s2, BYTE_LEN_128B);
#endif
	ptr = append(&tmpbuf[BYTE_LEN_128B], tmp, BYTE_LEN_128B);
	ptr = append(ptr, tmp1, PUBLIC_KEY_LEN);
	ptr = append(ptr, tmp2, PUBLIC_KEY_LEN);

	size = (int)(((unsigned long)ptr) - ((unsigned long)tmpbuf));
	hmac_sha256(tmpbuf, size, pSta->auth_key, BYTE_LEN_256B, tmp, &size);
	pMsg = add_tlv(pMsg, TAG_E_HASH2, BYTE_LEN_256B, (void *)tmp);
#ifdef DEBUG
	if (pCtx->debug2)
		debug_out("E-Hash2", tmp, BYTE_LEN_256B);
#endif

#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2)
		pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, 6, (void *)WSC_VENDOR_V2);
#endif


	size = (int)(((unsigned long)pMsg) - ((unsigned long)pMsgStart));
	append(&pSta->last_rx_msg[pSta->last_rx_msg_size], pMsgStart, size);
	hmac_sha256(pSta->last_rx_msg, pSta->last_rx_msg_size+size, pSta->auth_key, BYTE_LEN_256B, tmp, &size);
	pMsg = add_tlv(pMsg, TAG_AUTHENTICATOR, BYTE_LEN_64B, (void *)tmp);


	//================================================================	

	size = (int)(((unsigned long)pMsg) - ((unsigned long)pMsgStart));

#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2) {
#ifdef EAP_FRAGMENT
#ifdef SUPPORT_UPNP
		/*cond2,we don't frag when AP as enrollee,(by ER config) ; WPS2DOTX */ 
		if (!(pSta->used & IS_UPNP_CONTROL_POINT)  &&  (!pCtx->is_ap))  
#endif		
		{
			if(pCtx->EAP_frag_threshold){
				
				if(size > pCtx->EAP_frag_threshold){
					//pCtx->origeMgsSize=size;
					pCtx->Feap = eap ;
					pCtx->Feapol = eapol ;
					pCtx->Fwsc = wsc ;
					pSta->sendToSta_MegTotalSize = size ; 
					send_frag_msg(pCtx , pSta ,ST_WAIT_EAPOL_FRAG_ACK_M3 ,1 ); // first time EAP Frag
					return 0;
				}	
			}
		}
#endif	
	}
#endif

	
	memcpy(pSta->last_tx_msg_buffer, pMsgStart, size);
	pSta->last_tx_msg_size = size;

#ifdef SUPPORT_UPNP
	if (!(pSta->used & IS_UPNP_CONTROL_POINT)) 
#endif	
	{
		eapol->packet_body_length = htons(EAP_HDRLEN + sizeof(*wsc) + size);
		eap->length = htons(EAP_HDRLEN + sizeof(*wsc) + size);
		pSta->tx_size = ETHER_HDRLEN + EAPOL_HDRLEN + EAP_HDRLEN + sizeof(*wsc) + size;
	}

	DEBUG_PRINT("\n>> Sending EAP WSC_MSG M3\n");

#ifdef SUPPORT_UPNP
	if (pSta->used & IS_UPNP_CONTROL_POINT)
		return 0;
	else
#endif		
	{
		return send_wlan(pCtx, pSta->tx_buffer, pSta->tx_size);
	}
}

int send_wsc_M1(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	struct eapol_t *eapol;
	struct eap_t *eap;
	struct eap_wsc_t *wsc;
	int size;
	int TagSize;
    int idx=0;
	unsigned char *pMsg, byteVal, *pMsgStart;
	unsigned char tmpbuf[200];
	unsigned short shortVal;
	unsigned long longVal;
	DH *a;
	#ifdef WPS2DOTX
	unsigned short shortVal2;
	#endif	

	DBFENTER;

#ifdef SUPPORT_UPNP
	if (pSta->used & IS_UPNP_CONTROL_POINT) 
		pMsg = pSta->tx_buffer;
	else
#endif		
	{
		build_wsc_msg_header(pCtx, pSta, WSC_OP_MSG, &eapol, &eap, &wsc);	
		pMsg =	((unsigned char *)wsc) + sizeof(struct eap_wsc_t);
	}
	pMsgStart = pMsg;

	byteVal = WSC_VER;
	pMsg = add_tlv(pMsg, TAG_VERSION, 1, (void *)&byteVal);
	byteVal = MSG_TYPE_M1;
	pMsg = add_tlv(pMsg, TAG_MSG_TYPE, 1, (void *)&byteVal);
	pMsg = add_tlv(pMsg, TAG_UUID_E, UUID_LEN, (void *)pCtx->uuid);
#ifdef FOR_DUAL_BAND		
	if(pCtx->InterFaceComeIn == COME_FROM_WLAN1){
		pMsg = add_tlv(pMsg, TAG_MAC_ADDRESS, ETHER_ADDRLEN, (void *)pCtx->our_addr2);
	}
	else if(pCtx->InterFaceComeIn == COME_FROM_WLAN0){
		pMsg = add_tlv(pMsg, TAG_MAC_ADDRESS, ETHER_ADDRLEN, (void *)pCtx->our_addr);
	}
	else	{
		pMsg = add_tlv(pMsg, TAG_MAC_ADDRESS, ETHER_ADDRLEN, (void *)pCtx->our_addr);
	}
#else //FOR_DUAL_BAND
	pMsg = add_tlv(pMsg, TAG_MAC_ADDRESS, ETHER_ADDRLEN, (void *)pCtx->our_addr);
#endif //FOR_DUAL_BAND

	generate_random(pSta->nonce_enrollee, NONCE_LEN);
#ifdef DEBUG_PIXIE_DUST_ATTACK
	printf("[N1] nonce_e\n");
    for(idx=0;idx<NONCE_LEN;idx++)
        printf("%02x  ",pSta->nonce_enrollee[idx]);
    printf("\n");
	printf("[N1] nonce_e end\n");	
#endif    
	pMsg = add_tlv(pMsg, TAG_EROLLEE_NONCE, NONCE_LEN, (void *)pSta->nonce_enrollee);

	a = generate_dh_parameters(PUBLIC_KEY_LEN*8, wsc_prime_num, DH_GENERATOR_2);
	if (a == NULL)
		return -1;

	a->flags &= ~DH_FLAG_NO_EXP_CONSTTIME;

	if (!DH_generate_key(a)) {
		if (a){
			DH_free(a);
			a = NULL;
		}
		DEBUG_ERR("DH_generate_key(a) error!\n");
		return -1;		
	}

#ifdef DEBUG
	if (pCtx->debug2) {
	//	printf("pri 1=");
	//	bn_printxxxx(a->priv_key);
		_DEBUG_PRINT("\nDH public-key =");
		bn_printxxxx(a->pub_key);
		_DEBUG_PRINT("\n");	
	}
#endif	
	BN_bn2bin(a->pub_key, tmpbuf);
	pMsg = add_tlv(pMsg, TAG_PUB_KEY, PUBLIC_KEY_LEN, (void *)tmpbuf);
	pSta->dh_enrollee = a;

	shortVal = htons(((unsigned short)pCtx->auth_type_flags));
	pMsg = add_tlv(pMsg, TAG_AUTH_TYPE_FLAGS, 2, (void *)&shortVal);
	shortVal = htons(((unsigned short)pCtx->encrypt_type_flags));
	pMsg = add_tlv(pMsg, TAG_ENCRYPT_TYPE_FLAGS, 2, (void *)&shortVal);
	byteVal = (unsigned char)pCtx->connect_type;
	pMsg = add_tlv(pMsg, TAG_CONNECT_TYPE_FLAGS, 1, (void *)&byteVal);

	shortVal = htons(((unsigned short)pCtx->config_method));
	pMsg = add_tlv(pMsg, TAG_CONFIG_METHODS, 2, (void *)&shortVal);

	byteVal = (unsigned char)pCtx->config_state;	
	pMsg = add_tlv(pMsg, TAG_SIMPLE_CONFIG_STATE, 1, (void *)&byteVal);

	TagSize = strlen(pCtx->manufacturer);
	pMsg = add_tlv(pMsg, TAG_MANUFACTURER, TagSize, (void *)pCtx->manufacturer);

	TagSize = strlen(pCtx->model_name);
	pMsg = add_tlv(pMsg, TAG_MODEL_NAME, TagSize, (void *)pCtx->model_name);		

	TagSize = strlen(pCtx->model_num);
	pMsg = add_tlv(pMsg, TAG_MODEL_NUMBER, TagSize, (void *)pCtx->model_num);

	TagSize = strlen(pCtx->serial_num);	
	pMsg = add_tlv(pMsg, TAG_SERIAL_NUM, TagSize, (void *)pCtx->serial_num);

	shortVal = htons(((unsigned short)pCtx->device_category_id));
	memcpy(tmpbuf, &shortVal, 2);
	memcpy(&tmpbuf[2], pCtx->device_oui, OUI_LEN);
	shortVal = htons(((unsigned short)pCtx->device_sub_category_id));
	memcpy(&tmpbuf[6], &shortVal, 2);	
	pMsg = add_tlv(pMsg, TAG_PRIMARY_DEVICE_TYPE, 8, (void *)tmpbuf);


	TagSize = strlen(pCtx->device_name);	
	pMsg = add_tlv(pMsg, TAG_DEVICE_NAME, TagSize, (void *)pCtx->device_name);

#ifdef FOR_DUAL_BAND
	if(pCtx->is_ap){
		if(pCtx->InterFaceComeIn==COME_FROM_WLAN0)
			// use rf_band from wlan0's flash mib ;because wlan0 maybe 5g or 2.4g
			byteVal = (unsigned char)pCtx->rf_band;	
		else if(pCtx->InterFaceComeIn==COME_FROM_WLAN1)
			byteVal =1;
		else
			byteVal =1; // default use 2.4g
	}
	else{
		if(pCtx->STAmodeNegoWith == COMEFROM5G){
			byteVal =2;
		}
		else if(pCtx->STAmodeNegoWith == COMEFROM24G){
			byteVal =1;
		}
		else
		{
			byteVal =1; // default use 2.4g
		}
	}
#else //FOR_DUAL_BAND
	byteVal = (unsigned char)pCtx->rf_band;
#endif //FOR_DUAL_BAND

	pMsg = add_tlv(pMsg, TAG_RF_BAND, 1, (void *)&byteVal);
#if 0 //for debug MS
	shortVal = ASSOC_STATE_CONNECT_SUCCESS; // TODO:
#else
	shortVal = ASSOC_STATE_NOT_ASSOC; // TODO:
#endif
	shortVal = htons((unsigned short)shortVal);
	pMsg = add_tlv(pMsg, TAG_ASSOC_STATE, 2, (void *)&shortVal);	
	if ((!IS_PIN_METHOD(pCtx->config_method) && (pSta->device_password_id != PASS_ID_PB))
			|| pCtx->pb_pressed)
		shortVal = htons((unsigned short)PASS_ID_PB);		
	else
		shortVal = htons((unsigned short)pCtx->device_password_id);
	pMsg = add_tlv(pMsg, TAG_DEVICE_PASSWORD_ID, 2, (void *)&shortVal);
	shortVal = htons((unsigned short)pCtx->config_err);
	pMsg = add_tlv(pMsg, TAG_CONFIG_ERR, 2, (void *)&shortVal);
	longVal = htonl(pCtx->os_ver | 0x10000000);
	pMsg = add_tlv(pMsg, TAG_OS_VERSION, 4, (void *)&longVal);


#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2)
		pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, 6, (void *)WSC_VENDOR_V2);
#endif

	//================================================================	

	size = (int)(((unsigned long)pMsg) - ((unsigned long)pMsgStart));
#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2) {
#ifdef EAP_FRAGMENT
#ifdef SUPPORT_UPNP
		/*cond2,we don't frag when AP as enrollee,(by ER config) ; WPS2DOTX */ 
		if (!(pSta->used & IS_UPNP_CONTROL_POINT)  &&  (!pCtx->is_ap))  
#endif		
		{
			if(pCtx->EAP_frag_threshold){
				
				if(size > pCtx->EAP_frag_threshold){
					//pCtx->origeMgsSize=size;
					pCtx->Feap = eap ;
					pCtx->Feapol = eapol ;
					pCtx->Fwsc = wsc ;
					pSta->sendToSta_MegTotalSize = size ; 
					send_frag_msg(pCtx , pSta ,ST_WAIT_EAPOL_FRAG_ACK_M1 ,1 ); // first time EAP Frag
					return 0;
				}	
			}
		}
#endif	
	}
#endif


	memcpy(pSta->last_tx_msg_buffer, pMsgStart, size);
	pSta->last_tx_msg_size = size;
#ifdef SUPPORT_UPNP
	if (!(pSta->used & IS_UPNP_CONTROL_POINT)) 
#endif	
	{
		eapol->packet_body_length = htons(EAP_HDRLEN + sizeof(*wsc) + size);
		eap->length = htons(EAP_HDRLEN + sizeof(*wsc) + size);
		pSta->tx_size = ETHER_HDRLEN + EAPOL_HDRLEN + EAP_HDRLEN + sizeof(*wsc) + size;
	}

        report_WPS_STATUS(SEND_M1);
	DEBUG_PRINT("\n>> Sending EAP WSC_MSG M1\n");

#ifdef SUPPORT_UPNP
	if (pSta->used & IS_UPNP_CONTROL_POINT)
		return 0;
	else
#endif		
	{
		return send_wlan(pCtx, pSta->tx_buffer, pSta->tx_size);
	}	

}

int send_eap_rspid(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	struct eapol_t *eapol;
	struct eap_t *eap;
	unsigned char *packet;
	struct ethernet_t *eth_hdr;
	struct eap_rr_t * eaprr;
	char *nai;
	int size, nai_len;

	DBFENTER;

	if (pCtx->role == REGISTRAR) {
		nai = EAP_ID_REGISTRAR;
		nai_len = strlen(EAP_ID_REGISTRAR);
	}		
	else {
		nai = EAP_ID_ENROLLEE;
		nai_len = strlen(EAP_ID_ENROLLEE);
	}
	size = ETHER_HDRLEN + sizeof(struct eapol_t) + sizeof(struct eap_t ) + 
			sizeof(struct eap_rr_t) + nai_len;
	memset(pSta->tx_buffer, '\0', size);
	packet = pSta->tx_buffer;
	
	eth_hdr = (struct ethernet_t *)packet;
	memcpy(eth_hdr->ether_dhost, pSta->addr, ETHER_ADDRLEN);

#ifdef FOR_DUAL_BAND		
	if(pCtx->InterFaceComeIn == COME_FROM_WLAN1){
		memcpy(eth_hdr->ether_shost, pCtx->our_addr2, ETHER_ADDRLEN);		
	}
	else if(pCtx->InterFaceComeIn == COME_FROM_WLAN0){
		memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);		
	}
	else	{
		memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);		
	}
#else //FOR_DUAL_BAND
	memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);
#endif //FOR_DUAL_BAND
	eth_hdr->ether_type = htons(ETHER_EAPOL_TYPE);
		
	eapol = (struct eapol_t *)(packet + ETHER_HDRLEN);
	eapol->protocol_version = EAPOL_VER;
	eapol->packet_type = EAPOL_EAPPKT;
	eapol->packet_body_length = htons(EAP_HDRLEN  + sizeof(struct eap_rr_t) + nai_len);
	
	eap = (struct eap_t *)(((unsigned char *)eapol) + EAPOL_HDRLEN);
	eap->code =  EAP_RESPONSE;
	eap->identifier = pSta->eap_reqid;
	eap->length = htons(EAP_HDRLEN + sizeof(struct eap_rr_t) + nai_len);
	
	eaprr = (struct eap_rr_t *)(((unsigned char *)eapol) + EAPOL_HDRLEN  + EAP_HDRLEN);
	eaprr->type =  EAP_TYPE_IDENTITY;

	memcpy((char *)(((unsigned char *)eaprr)+sizeof(struct eap_rr_t)), nai, nai_len);

	pSta->tx_size = size;

        report_WPS_STATUS(RECV_EAP_IDRSP);
	DEBUG_PRINT("\n>> Sending EAP RESPONSE / Identity packet\n");

	return send_wlan(pCtx, pSta->tx_buffer, pSta->tx_size);
}

int send_eapol_start(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	struct eapol_t *eapol;
	unsigned char *packet;
	struct ethernet_t *eth_hdr;
	int size;

	DBFENTER;

	size = ETHER_HDRLEN + sizeof(struct eapol_t);
	memset(pSta->tx_buffer, '\0', size);
	packet = pSta->tx_buffer;

	eth_hdr = (struct ethernet_t *)packet;
	memcpy(eth_hdr->ether_dhost, pSta->addr, ETHER_ADDRLEN);

#ifdef FOR_DUAL_BAND		
	if(pCtx->InterFaceComeIn == COME_FROM_WLAN1){
		memcpy(eth_hdr->ether_shost, pCtx->our_addr2, ETHER_ADDRLEN);		
	}
	else if(pCtx->InterFaceComeIn == COME_FROM_WLAN0){
		memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);		
	}
	else	{
		memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);		
	}
#else //FOR_DUAL_BAND
	memcpy(eth_hdr->ether_shost, pCtx->our_addr, ETHER_ADDRLEN);
#endif //FOR_DUAL_BAND
	eth_hdr->ether_type = htons(ETHER_EAPOL_TYPE);

	eapol = (struct eapol_t *)(packet + ETHER_HDRLEN);
	eapol->protocol_version = EAPOL_VER;
	eapol->packet_type = EAPOL_START;
	eapol->packet_body_length = htons(0);

	report_WPS_STATUS(SEND_EAPOL_START);
	
	_DEBUG_PRINT("\n>> Sending EAPOL-Start packet\n");

	pSta->tx_size = size;

	return send_wlan(pCtx, pSta->tx_buffer, pSta->tx_size);	
}

#endif // SUPPORT_ENROLLEE


#ifdef SUPPORT_UPNP
int send_upnp_to_wlan(CTX_Tp pCtx, STA_CTX_Tp pSta, struct WSC_packet *packet)
{
	struct eapol_t *eapol;
	struct eap_t *eap;
	struct eap_wsc_t *wsc;
	unsigned char *pMsg;

	DBFENTER;

		
#ifdef DEBUG				
#ifdef FOR_DUAL_BAND		
	if(pCtx->debug2 && pCtx->is_ap){
		if(pCtx->InterFaceComeIn == COME_FROM_WLAN1){
			TX_DEBUG("wlan1\n");
			MAC_PRINT(pCtx->our_addr2);
		}
		else if(pCtx->InterFaceComeIn == COME_FROM_WLAN0){
			TX_DEBUG("wlan0\n");
			MAC_PRINT(pCtx->our_addr);
		}
	}
#endif //FOR_DUAL_BAND
#endif

	build_wsc_msg_header(pCtx, pSta, WSC_OP_MSG, &eapol, &eap, &wsc);	
	
	pMsg =	((unsigned char *)wsc) + sizeof(struct eap_wsc_t);
	memcpy(pMsg, packet->rx_buffer, packet->rx_size);
	
	eapol->packet_body_length = htons(EAP_HDRLEN + sizeof(*wsc) + packet->rx_size);	
	eap->length = htons(EAP_HDRLEN + sizeof(*wsc) + packet->rx_size);	
	pSta->tx_size = ETHER_HDRLEN + EAPOL_HDRLEN + EAP_HDRLEN + sizeof(*wsc) + packet->rx_size;

	_DEBUG_PRINT(">> Forward UPNP msg to STA\n");
	MAC_PRINT(pSta->addr);

	return send_wlan(pCtx, pSta->tx_buffer, pSta->tx_size);
}
#endif // SUPPORT_UPNP




