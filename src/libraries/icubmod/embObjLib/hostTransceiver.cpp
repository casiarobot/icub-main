
/* @file       hostTransceiver.c
    @brief      This file implements internal implementation of a nv object.
    @author     marco.accame@iit.it
    @date       09/03/2010
**/


// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

using namespace std;

#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"

#include "string.h"
#include "hostTransceiver.hpp"
#include <yarp/os/impl/Logger.h>

#include "EoCommon.h"
#include "EOnv_hid.h"

#define _DEBUG_ON_FILE_

#ifdef _DEBUG_ON_FILE_
	#define SOGLIA						70000
	#define MAX_ACQUISITION 			1000
	uint64_t idx = 0;
	uint64_t max_idx = MAX_ACQUISITION*7*16*2 / 8;
	uint64_t utime[MAX_ACQUISITION*16*7*2*2] = {0};
	uint64_t errors[MAX_ACQUISITION*16*7*2*2][2] = {0};
	uint64_t nErr=0;
	FILE *outFile = NULL;
#endif

// mutex
//#include <pthread.h>

using namespace yarp::os::impl;

// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------

// originally defined here as a global static -- this prevent instantiation of one of those objects (I need one x ems)
//static hostTransceiver thehosttransceiver;

//static EOhostTransceiver *hosttxrx = NULL;
//static EOtransceiver *pc104txrx = NULL;
//static EOnvsCfg *pc104nvscfg = NULL:


// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------


hostTransceiver::hostTransceiver()
{
	YARP_INFO(Logger::get(), "hostTransceiver::hostTransceiver()", Logger::get().log_files.f3);
}

hostTransceiver::~hostTransceiver()
{

}

void hostTransceiver::init(uint32_t _localipaddr, uint32_t _remoteipaddr, uint16_t _ipport, uint16_t _pktsize, uint8_t _board_n)
{
	YARP_INFO(Logger::get(),"hostTransceiver::init", Logger::get().log_files.f3);

    // the configuration of the transceiver: it is specific of a given remote board

    eOhosttransceiver_cfg_t hosttxrxcfg;
    hosttxrxcfg.remoteboardipv4addr   	= 	_remoteipaddr;
    hosttxrxcfg.remoteboardipv4port 	=  	_ipport;
    hosttxrxcfg.tobedefined 			=   0;


    switch(_board_n)
    {
    	case 1:
    		hosttxrxcfg.vectorof_endpoint_cfg = eo_cfg_EPs_vectorof_eb1;
    		hosttxrxcfg.hashfunction_ep2index = eo_cfg_nvsEP_eb1_fptr_hashfunction_ep2index;
    		break;
    	case 2:
    		hosttxrxcfg.vectorof_endpoint_cfg = eo_cfg_EPs_vectorof_eb2;
    		hosttxrxcfg.hashfunction_ep2index = eo_cfg_nvsEP_eb2_fptr_hashfunction_ep2index;
    		break;
    	case 3:
    		hosttxrxcfg.vectorof_endpoint_cfg = eo_cfg_EPs_vectorof_eb3;
    		hosttxrxcfg.hashfunction_ep2index = eo_cfg_nvsEP_eb3_fptr_hashfunction_ep2index;
    		break;
    	case 4:
    		hosttxrxcfg.vectorof_endpoint_cfg = eo_cfg_EPs_vectorof_eb4;
    		hosttxrxcfg.hashfunction_ep2index = eo_cfg_nvsEP_eb4_fptr_hashfunction_ep2index;
    		break;
    	case 5:
    		hosttxrxcfg.vectorof_endpoint_cfg = eo_cfg_EPs_vectorof_eb5;
    		hosttxrxcfg.hashfunction_ep2index = eo_cfg_nvsEP_eb5_fptr_hashfunction_ep2index;
    		break;
    	case 6:
    		hosttxrxcfg.vectorof_endpoint_cfg = eo_cfg_EPs_vectorof_eb6;
    		hosttxrxcfg.hashfunction_ep2index = eo_cfg_nvsEP_eb6_fptr_hashfunction_ep2index;
    		break;
    	case 7:
    		hosttxrxcfg.vectorof_endpoint_cfg = eo_cfg_EPs_vectorof_eb7;
    		hosttxrxcfg.hashfunction_ep2index = eo_cfg_nvsEP_eb7_fptr_hashfunction_ep2index;
    		break;
    	case 8:
    		hosttxrxcfg.vectorof_endpoint_cfg = eo_cfg_EPs_vectorof_eb8;
    		hosttxrxcfg.hashfunction_ep2index = eo_cfg_nvsEP_eb8_fptr_hashfunction_ep2index;
    		break;
    	case 9:
    		hosttxrxcfg.vectorof_endpoint_cfg = eo_cfg_EPs_vectorof_eb9;
    		hosttxrxcfg.hashfunction_ep2index = eo_cfg_nvsEP_eb9_fptr_hashfunction_ep2index;
    		break;
	}

    localipaddr  = _localipaddr;
    remoteipaddr = _remoteipaddr;
    ipport       = _ipport;
    
    // initialise the transceiver: it creates a EOtransceiver and its nvsCfg by loading all the endpoints
    hosttxrx     = eo_hosttransceiver_New(&hosttxrxcfg);
    
    // retrieve teh transceiver
    pc104txrx    = eo_hosttransceiver_Transceiver(hosttxrx);
    
    // retrieve the nvscfg
    pc104nvscfg  = eo_hosttransceiver_NVsCfg(hosttxrx);

    pktTx = eo_packet_New(_pktsize);
    pktRx = eo_packet_New(_pktsize);
}

void hostTransceiver::load_occasional_rop(eOropcode_t opc, uint16_t ep, uint16_t nvid)
{
	//pc104txrx
    eo_transceiver_ropinfo_t ropinfo;

    ropinfo.ropcfg      = eok_ropconfig_basic;
    ropinfo.ropcode     = opc;
    ropinfo.nvep        = ep;

    ropinfo.nvid = nvid;
    eOresult_t res = eo_transceiver_rop_occasional_Load(pc104txrx, &ropinfo);
    printf("load_occasional_rop res = 0x%0X\n", res);
}

void hostTransceiver::s_eom_hostprotoc_extra_protocoltransceiver_configure_regular_rops_on_board(void)
{
#if 0
    EOarray *upto10 = (EOarray*) & eo_cfg_nvsEP_mngmnt_usr_rem_board_mem_local->upto10rop2signal;
    eOropSIGcfg_t sigcfg;
    char str[128];
    static uint8_t reset = 0;

    eo_array_Reset(upto10);

    if(0 == reset)
    {

        sigcfg.ep = EOK_cfg_nvsEP_base_endpoint;
        sigcfg.id = EOK_cfg_nvsEP_base_NVID_ipnetwork;
        sigcfg.plustime = 1;
        eo_array_PushBack(upto10, &sigcfg);


        sigcfg.ep = EOK_cfg_nvsEP_base_endpoint;
        sigcfg.id = EOK_cfg_nvsEP_base_NVID__bootprocess;
        sigcfg.plustime = 1;
        eo_array_PushBack(upto10, &sigcfg);

        sigcfg.ep = EOK_cfg_nvsEP_base_endpoint;
        sigcfg.id = EOK_cfg_nvsEP_base_NVID__applicationinfo;
        sigcfg.plustime = 0;
        eo_array_PushBack(upto10, &sigcfg);

        sigcfg.ep = EOK_cfg_nvsEP_base_endpoint;
        sigcfg.id = EOK_cfg_nvsEP_base_NVID__boardinfo;
        sigcfg.plustime = 0;
        eo_array_PushBack(upto10, &sigcfg);

        sigcfg.ep = EOK_cfg_nvsEP_base_endpoint;
        sigcfg.id = EOK_cfg_nvsEP_base_NVID_ipnetwork__ipnetmask;
        sigcfg.plustime = 0;
        eo_array_PushBack(upto10, &sigcfg);

        sigcfg.ep = EOK_cfg_nvsEP_base_endpoint;
        sigcfg.id = EOK_cfg_nvsEP_base_NVID_ipnetwork__ipaddress;
        sigcfg.plustime = 0;
        eo_array_PushBack(upto10, &sigcfg);

        sigcfg.ep = EOK_cfg_nvsEP_base_endpoint;
        sigcfg.id = EOK_cfg_nvsEP_base_NVID__remoteipaddress;
        sigcfg.plustime = 0;
        eo_array_PushBack(upto10, &sigcfg);

        sigcfg.ep = EOK_cfg_nvsEP_base_endpoint;
        sigcfg.id = EOK_cfg_nvsEP_base_NVID__remoteipport;
        sigcfg.plustime = 0;
        eo_array_PushBack(upto10, &sigcfg);

    }

    load_occasional_rop(eo_ropcode_set, EOK_cfg_nvsEP_mngmnt_endpoint, EOK_cfg_nvsEP_mngmnt_NVID__upto10rop2signal);

    if(1 == reset)
    {
        printf( "added a set<__upto10rop2signal, empty-list>");
    }
    else
    {
        printf("added a set<__upto10rop2signal, list>");
    }

    reset = (0==reset) ? (1) : (0);
#endif
}

// vecchio //
void hostTransceiver::hostTransceiver_ConfigureRegularsOnRemote(void)
{
    // 1. at first write the nv that we want to set: the EOK_cfg_nvsEP_mngmnt_NVID__upto15epid2signal
    //    there are two alternative ways to do that:
    //    A. QUICK MODE - to write directly onto the local struct of the endpoint. WE USE THIS SECOND WAY because we know
    //       the name of the specific variable to use in the rop
    //    B. FORMAL MODE (and more generic): to use teh nvscfg to retrieve the handle of a EOnv which is relative to the 
    //       pair (endpoint, id) which we wnat to manipulate, and then to use the methods of EOnv to set the value of that variable.
    //       we shall use this mode in function hostTransceiver_AddSetROP()
    
//    EOarray *upto15 = (EOarray*) & eo_cfg_nvsEP_mngmnt_usr_rem_board_mem_local->upto10rop2signal;
#warning "cambiato?? per compilare scommentato definizione eOnvEPID_t in EOnv.h"
//    eOnvEPID_t epid;

    // clear the variable.
//    eo_array_Reset(upto15);

    // push back in teh array the first nv that the remote board shall signal
//    epid.ep = EOK_cfg_nvsEP_base_endpoint;
//    epid.id = EOK_cfg_nvsEP_base_NVID_ipnetwork__ipaddress;
//    eo_array_PushBack(upto15, &epid);

    // push back in teh array the second nv that the remote board shall signal
//    epid.ep = EOK_cfg_nvsEP_base_endpoint;
//    epid.id = EOK_cfg_nvsEP_base_NVID__bootprocess;
//    eo_array_PushBack(upto15, &epid);
    
    // 2. then put the rop set<upto15epid2signal, data> inside the transceiver
//    s_hostTransceiver_AddSetROP_with_data_already_set(EOK_cfg_nvsEP_mngmnt_endpoint, EOK_cfg_nvsEP_mngmnt_NVID__upto10rop2signal);
}


// somebody adds a set-rop  plus data.
void hostTransceiver::hostTransceiver_AddSetROP(uint16_t ep, uint16_t id, uint8_t* data, uint16_t size)
{
    uint16_t ss;
    EOnv nv;

    // 1. search for teh EOnv with pair (ep, id)
            
 //   if(eores_OK != eo_nvscfg_GetNV(thehosttransceiver.pc104nvscfg, thehosttransceiver.remoteipaddr, ep, id, &nv))
    {
        // there is no such variable with (remoteipaddr-ep-id) 
 //       return;
    }
    
    // 1bis. verify that the datasize is correct.
    ss = eo_nv_Size(&nv, data);
    if(ss < size)
    {
        // non faccio nulla intanto quando scrivo uso la capacita' della nv.
        // sarebbe bene pero' emettere un warning        
    }
    
    
    // 2. set the data. dont force the set so that we dont write if the nv is readonly.
    
    if(eores_OK != eo_nv_Set(&nv, data, eobool_false, eo_nv_upd_dontdo))
    {   
        // teh nv is not writeable
        return;
    }
    
    
    // 3. add the rop 
    s_hostTransceiver_AddSetROP_with_data_already_set(ep, id);

}


// somebody passes the received packet - this is used just as an interface
void hostTransceiver::onMsgReception(uint8_t *data, uint16_t size)
{
	//this->pkt = data;
	SetReceived(data, size);
}

// and Processes it
void hostTransceiver::SetReceived(uint8_t *data, uint16_t size)
{
    uint16_t numofrops;
    uint64_t txtime;
    static uint32_t prevTime = 0;
    static uint32_t prevNum = 0;
    uint32_t progNum;

    eo_packet_Payload_Set(pktRx, data, size);
    eo_packet_Addressing_Set(pktRx, remoteipaddr, ipport);
    eo_transceiver_Receive(pc104txrx, pktRx, &numofrops, &txtime);

#ifdef _DEBUG_ON_FILE_
    if(idx == 0)
    {
    	utime[idx] = 0;
    	progNum = (txtime >> 32);
        prevTime = (txtime & 0xFFFFFFFF);
        prevNum = progNum;
        idx++;
    }

    else if(idx < max_idx)
	{
    	utime[idx] = (txtime & 0xFFFFFFFF);// - prevTime;
    	progNum = (txtime >> 32);

//        if(utime[idx] >= SOGLIA)
//        	printf("utime - prevTime = %d\n ",utime[idx] - prevTime);
        if( (progNum - prevNum) > 1)
        {
        	errors[nErr][0] =  prevNum;
			errors[nErr][1] =  progNum;
	        printf("missing packet - old = %d, new = %d\n ",prevNum, progNum);
        	printf("missing packet - old = %d, new = %d\n ",errors[nErr][0], errors[nErr][1]);
        	nErr++;
        }

        prevTime = (txtime & 0xFFFFFFFF);
        prevNum = progNum;
        idx++;
	}

    if(idx == max_idx)
    {
    	outFile = fopen("/usr/local/src/robot/pc104-logs/transceiverTimestamp.txt", "w+");
    	printf("Trans fopen: %s\n",strerror(errno));
    	if(NULL == outFile)
    	{
    		outFile = stdout;
    	}
//    	for(uint64_t i=0; i<idx; i++)
//    		fprintf(outFile, "%d\n", utime[i]);

    	fprintf(outFile, "\n\nMissing packets: %d over %d\n\n", nErr, max_idx);
    	uint64_t diff;
    	for(uint64_t i=0; i< nErr; i++)
    	{
    		diff = errors[i][1] - errors[i][0];
    		fprintf(outFile, "%d -> %d (%d)\n", errors[i][0] , errors[i][1], (uint32_t) diff);
    	}
    	printf("Trans fprintf: %s\n",strerror(errno));

    	if(outFile != stdout)
    		fclose(outFile);
        idx++;
    }

#endif
}

// somebody retrieves what must be transmitted
void hostTransceiver::getTransmit(uint8_t **data, uint16_t *size)
{
    uint16_t numofrops;
    
    eo_transceiver_Transmit(pc104txrx, &pktTx, &numofrops);
    
    eo_packet_Payload_Get(pktTx, data, size);
    
}



// --------------------------------------------------------------------------------------------------------------------
// - definition of extern hidden functions 
// --------------------------------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------

void hostTransceiver::s_hostTransceiver_AddSetROP_with_data_already_set(uint16_t ep, uint16_t id)
{
    eo_transceiver_ropinfo_t ropinfo;

    ropinfo.ropcfg      = eok_ropconfig_basic;
    ropinfo.ropcode     = eo_ropcode_set;
    ropinfo.nvep        = ep;
    ropinfo.nvid        = id;
    
    eo_transceiver_rop_occasional_Load(pc104txrx, &ropinfo);
}

void hostTransceiver::s_hostTransceiver_AddGetROP(uint16_t ep, uint16_t id)
{
    eo_transceiver_ropinfo_t ropinfo;

    ropinfo.ropcfg      = eok_ropconfig_basic;
    ropinfo.ropcode     = eo_ropcode_ask;
    ropinfo.nvep        = ep;
    ropinfo.nvid        = id;

    eo_transceiver_rop_occasional_Load(pc104txrx, &ropinfo);
}

// meglio nn usare?? il meccanismo di wait con tabella fare nel embObjMotCtrl anzichè qui
void hostTransceiver::askNV(uint16_t endpoint, uint16_t id, uint8_t* data, uint16_t* size)
{
	// add the rop to the ropframe
	load_occasional_rop(eo_ropcode_ask, endpoint, id);

	// wait fot the packet to be sent and for the reply to reach me!!
	EOnv	*nv = getNVhandler( endpoint,  id);


	//pthread_mutex_t mutex;
	//pthread_mutex_init(&mutex, NULL);
	//pthread_mutex_lock(&mutex);

	// now, get the value
	//getNVvalue(nv, data, size);
}

EOnv* hostTransceiver::getNVhandler(uint16_t endpoint, uint16_t id)
{
//	YARP_INFO(Logger::get(),"hostTransceiver::getNVvalue", Logger::get().log_files.f3);

	uint16_t		ondevindex = 0, onendpointindex = 0 , onidindex = 0;
	EOtreenode	*nvTreenodeRoot;
	EOnv 		*nvRoot=NULL;
	EOnv		tmp;
    eOresult_t res;

    // convetire come parametro, oppure mettere direttam l'indirizzo ip come parametro...
    eOnvOwnership_t	nvownership = eo_nv_ownership_remote;

	res = eo_nvscfg_GetIndices(	this->pc104nvscfg, remoteipaddr, endpoint, id, &ondevindex,	&onendpointindex, &onidindex);

	// if the nvscfg does not have the triple (ip, ep, id) then we return an error
	if(eores_OK != res)
	{
		// Do something about this case
		YARP_ERROR(Logger::get(), " WTF!! NV not found!!!\n",  Logger::get().log_files.f3);
	}
	else
	{
		// 2 passaggi e copia della nv
		// we need a treenode of the nv
		//nvTreenodeRoot = eo_nvscfg_GetTreeNode(this->pc104nvscfg, ondevindex, onendpointindex, onidindex);  //  ?? non usato??
		// but also the handle to the nv.
		//eo_nvscfg_GetNV(this->pc104nvscfg, ondevindex, onendpointindex, onidindex, nvTreenodeRoot, &nvRoot);

		// più diretta e restituisce solo il puntatore che è ciò che mi serve qui, ma non ho accesso a nvsCfg_allnvs
		//nvRoot = eo_matrix3d_At(pc104nvscfg->allnvs, ondevindex, onendpointindex, onidindex);
		nvRoot = eo_nvscfg_GetNV(this->pc104nvscfg, ondevindex, onendpointindex, onidindex, nvTreenodeRoot, &tmp);

		//getNVvalue(nvRoot, data, size);
	}
	return(nvRoot);
}


void hostTransceiver::getNVvalue(EOnv *nvRoot, uint8_t* data, uint16_t* size)
{
	YARP_INFO(Logger::get(),"hostTransceiver::getNVvalue", Logger::get().log_files.f3);

    if (NULL != nvRoot)
    {
		// get te actual value
		eo_nv_remoteGet(nvRoot, data, size);
	}
}








// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------




