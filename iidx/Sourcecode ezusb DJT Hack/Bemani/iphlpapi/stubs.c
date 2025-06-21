/* IPHLPAPI Stubs for libavs-win32 */
#include <windows.h>
#include <iphlpapi.h>
#include "libutil/util.h"

DWORD STDCALL get_adapters_info(IP_ADAPTER_INFO *a, ULONG *psz)
{
	if (*psz < sizeof(IP_ADAPTER_INFO)) {
		*psz = sizeof(IP_ADAPTER_INFO);
		return ERROR_BUFFER_OVERFLOW;
	} else {
		ZeroMemory(a, sizeof(IP_ADAPTER_INFO));

		memset(a->Address, 0xFF, 6);
		strcpy(a->DhcpServer.IpAddress.String, "192.168.1.1");
		strcpy(a->GatewayList.IpAddress.String, "192.168.1.1");
		strcpy(a->IpAddressList.IpAddress.String, "192.168.1.100");

		a->AddressLength = 6;
		a->DhcpEnabled = 1;
		a->LeaseExpires= -1;
		a->LeaseObtained = 1;
		a->Type = MIB_IF_TYPE_ETHERNET;

		return ERROR_SUCCESS;
	}
}

DWORD STDCALL get_if_table(MIB_IFTABLE *ift, ULONG *psz, BOOL sort)
{
	size_t nbytes;

	nbytes = sizeof(MIB_IFTABLE) + sizeof(MIB_IFROW);

	if (*psz < nbytes) {
		*psz = nbytes;
		return ERROR_INSUFFICIENT_BUFFER;
	} else {
		memset(ift->table[0].bPhysAddr, 0xFF, 6);

		ift->dwNumEntries = 0;
		ift->table[0].dwAdminStatus = 1;
		ift->table[0].dwMtu = 1024;
		ift->table[0].dwOperStatus = IF_OPER_STATUS_CONNECTED;
		ift->table[0].dwPhysAddrLen = 6;
		ift->table[0].dwSpeed = 100000;
		ift->table[0].dwType = IF_TYPE_ETHERNET_CSMACD;

		return ERROR_SUCCESS;
	}
}

DWORD STDCALL get_network_params(FIXED_INFO *inf, ULONG *psz)
{
	if (*psz < sizeof(FIXED_INFO)) {
		*psz = sizeof(FIXED_INFO);
		return ERROR_BUFFER_OVERFLOW;
	} else {
		ZeroMemory(inf, sizeof(FIXED_INFO));
		
		strcpy(inf->DnsServerList.IpAddress.String, "192.168.1.1");
		strcpy(inf->DomainName, "DOMAIN");
		strcpy(inf->HostName, "DJHACKERS");

		inf->EnableDns = 1;
		inf->NodeType = PEER_TO_PEER_NODETYPE;

		return ERROR_SUCCESS;
	}
}

DWORD STDCALL add_ip_address(IPAddr address, IPMask ipMask, DWORD ifIndex,
	ULONG *nteContext, ULONG *nteInstance)
{
	*nteContext = 0;
	*nteInstance = 0;

	return ERROR_SUCCESS;
}

DWORD STDCALL delete_ip_address(ULONG nteContext)
{
	return ERROR_SUCCESS;
}
