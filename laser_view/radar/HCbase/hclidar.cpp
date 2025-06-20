﻿#include "hclidar.h"
#include <vector>
#include <set>
#include <algorithm>
#include <stdio.h>
#include <numeric>
#include <sstream>
#include <fstream>
//#include <eigen3/Eigen/Eigen>

HCLidar::HCLidar()
{
	initPara();
	int iReadTimeoutms = 2; // 10
    bool bPollMode = true;
    bool bDistQ2 = false;
    bool bLoop = false;
	setCircleDataMode();
    bool rtn = initialize(D2A8, iReadTimeoutms, bDistQ2, bLoop, bPollMode);
    //m_p8Buff = new UCHAR[READ_BUFF_SIZE];
}

HCLidar::~HCLidar()
{
    m_lstBuff.clear();
    m_lstTemp.clear();
}

void HCLidar::initPara()
{
	m_iSDKStatus = SDK_UNINIT;
	m_bScanning = false;
	m_iLastErrorCode = 0;
	m_bDisconnect = true;
	m_bHadID = false;
	m_bGetIDTimeOut = false;
	m_bHadFact = false;
	m_bGetFactTimeOut = false;
	m_bInitTimeout = false;
	m_sStatistic.reset();
	m_bCheckSpeed = false;
}


void HCLidar::lidarReConnect()
{
	m_bInitTimeout = false;
    m_bHadID=false;
    m_bGetIDTimeOut=false;
    m_bHadFact=false;
    m_bGetFactTimeOut=false;
}

std::string HCLidar::getLidarID()
{
	if (m_bFactoryMode)
	{
		if (m_strLidarModel == X2N)
			return m_strDevID;

		if (m_bHadID)
			return m_strDevID;
		else
		{
			return "";
		}
	}
	else
	{
		return m_strDevID;
	}
	
		
}


void HCLidar::setWorkPara(tsSDKPara& sSDKPara)
{
	m_sSDKPara = sSDKPara;
}

bool HCLidar::setLidarPara(const char* chLidarModel)
{
    m_strLidarModel = chLidarModel;
    if(m_strLidarModel == X1B)
    {
		m_sAttr.dAngleOffsetD = 21;
		m_sAttr.dBaseline_mm = 20;
        m_sAttr.iFPSMax = FPS_2000_MAX;
        m_sAttr.iFPSMin = FPS_2000_MIN;
        m_sAttr.iSpeedMax = SPEED_312_MAX;
        m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
    }
    else if(m_strLidarModel == X1D)
    {
        m_sAttr.dAngleOffsetD = 21;
        m_sAttr.dBaseline_mm = 20;
        m_sAttr.iFPSMax = FPS_2000_MAX;
        m_sAttr.iFPSMin = FPS_2000_MIN;
        m_sAttr.iSpeedMax = SPEED_312_MAX;
        m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
    }
    else if(m_strLidarModel == X1E)
    {
        m_sAttr.dAngleOffsetD = 21;
        m_sAttr.dBaseline_mm = 20;
        m_sAttr.iFPSMax = FPS_2000_MAX;
        m_sAttr.iFPSMin = FPS_2000_MIN;
        m_sAttr.iSpeedMax = SPEED_312_MAX;
        m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
    }
    else if(m_strLidarModel == X1F)
    {
        m_sAttr.dAngleOffsetD = 21;
        m_sAttr.dBaseline_mm = 20;
        m_sAttr.iFPSMax = FPS_2000_MAX;
        m_sAttr.iFPSMin = FPS_2000_MIN;
        m_sAttr.iSpeedMax = SPEED_312_MAX;
        m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
    }
	else if (m_strLidarModel == X1G)
	{
		m_sAttr.dAngleOffsetD = 21;
		m_sAttr.dBaseline_mm = 20;
		m_sAttr.iFPSMax = FPS_3000_MAX;
		m_sAttr.iFPSMin = FPS_3000_MIN;
		m_sAttr.iSpeedMax = SPEED_312_MAX;
		m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_3000_NOR;
	}
	else if (m_strLidarModel == X1K)
	{
		m_sAttr.dAngleOffsetD = 21;
		m_sAttr.dBaseline_mm = 20;
		m_sAttr.iFPSMax = FPS_2000_MAX;
		m_sAttr.iFPSMin = FPS_2000_MIN;
		m_sAttr.iSpeedMax = SPEED_312_MAX;
		m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
	}
	else if (m_strLidarModel == X1L)
	{
		m_sAttr.dAngleOffsetD = 21;
		m_sAttr.dBaseline_mm = 20;
		m_sAttr.iFPSMax = FPS_2000_MAX;
		m_sAttr.iFPSMin = FPS_2000_MIN;
		m_sAttr.iSpeedMax = SPEED_312_MAX;
		m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
	}
    else if(m_strLidarModel == X1M)
    {
        m_sAttr.dAngleOffsetD = 21;
        m_sAttr.dBaseline_mm = 20;
        m_sAttr.iFPSMax = FPS_2000_MAX;
        m_sAttr.iFPSMin = FPS_2000_MIN;
        m_sAttr.iSpeedMax = SPEED_312_MAX;
        m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
    }
	else if (m_strLidarModel == X1N)
	{
		m_sAttr.dAngleOffsetD = 21;
		m_sAttr.dBaseline_mm = 20;
		m_sAttr.iFPSMax = FPS_3000_MAX;
		m_sAttr.iFPSMin = FPS_3000_MIN;
		m_sAttr.iSpeedMax = SPEED_312_MAX;
		m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_3000_NOR;
	}
	else if (m_strLidarModel == X1S)
	{
		m_sAttr.dAngleOffsetD = 21;
		m_sAttr.dBaseline_mm = 20;
		m_sAttr.iFPSMax = FPS_1800_MAX;
		m_sAttr.iFPSMin = FPS_1800_MIN;
		m_sAttr.iSpeedMax = SPEED_300_MAX;
		m_sAttr.iSpeedMin = SPEED_300_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_1800;
		m_sAttr.dCirclePoints = CICRLE_MAX_1800;
		m_sAttr.u64TSStepNs = 1e9 / FPS_1800_NOR;
	}
    else if(m_strLidarModel == X2A)
    {
        m_sAttr.dAngleOffsetD = 28.5;
        m_sAttr.dBaseline_mm = 17.92;
        m_sAttr.iFPSMax = FPS_3000_MAX;
        m_sAttr.iFPSMin = FPS_3000_MIN;
        m_sAttr.iSpeedMax = SPEED_360_MAX;
        m_sAttr.iSpeedMin = SPEED_360_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_3000;
		m_sAttr.dCirclePoints = CICRLE_MAX_3000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_3000_NOR;
    }
    else if(m_strLidarModel == X2B)
    {
        m_sAttr.dAngleOffsetD = 28.5;
        m_sAttr.dBaseline_mm = 17.92;
        m_sAttr.iFPSMax = FPS_2000_MAX;
        m_sAttr.iFPSMin = FPS_2000_MIN;
        m_sAttr.iSpeedMax = SPEED_312_MAX;
        m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
    }
    else if(m_strLidarModel == X2C)
    {
        m_sAttr.dAngleOffsetD = 28.5;
        m_sAttr.dBaseline_mm = 17.92;
        m_sAttr.iFPSMax = FPS_3000_MAX;
        m_sAttr.iFPSMin = FPS_3000_MIN;
        m_sAttr.iSpeedMax = SPEED_360_MAX;
        m_sAttr.iSpeedMin = SPEED_360_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_3000;
		m_sAttr.dCirclePoints = CICRLE_MAX_3000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_3000_NOR;
    }
    else if(m_strLidarModel == X2E)
    {
        m_sAttr.dAngleOffsetD = 28.5;
        m_sAttr.dBaseline_mm = 17.92;
        m_sAttr.iFPSMax = FPS_3000_MAX;
        m_sAttr.iFPSMin = FPS_3000_MIN;
        m_sAttr.iSpeedMax = SPEED_360_MAX;
        m_sAttr.iSpeedMin = SPEED_360_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_3000;
		m_sAttr.dCirclePoints = CICRLE_MAX_3000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_3000_NOR;
    }
    else if(m_strLidarModel == X2F)
    {
        m_sAttr.dAngleOffsetD = 28.5;
        m_sAttr.dBaseline_mm = 17.92;
        m_sAttr.iFPSMax = FPS_3000_MAX;
        m_sAttr.iFPSMin = FPS_3000_MIN;
        m_sAttr.iSpeedMax = SPEED_360_MAX;
        m_sAttr.iSpeedMin = SPEED_360_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_3000;
		m_sAttr.dCirclePoints = CICRLE_MAX_3000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_3000_NOR;
    }
    else if(m_strLidarModel == X2M)
    {
        m_sAttr.dAngleOffsetD = 28.5;
        m_sAttr.dBaseline_mm = 17.92;
        m_sAttr.iFPSMax = FPS_2000_MAX;
        m_sAttr.iFPSMin = FPS_2000_MIN;
        m_sAttr.iSpeedMax = SPEED_312_MAX;
        m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
    }
    else if(m_strLidarModel == X2N)
    {

        m_sAttr.dAngleOffsetD = 28.5;
        m_sAttr.dBaseline_mm = 17.92;
        m_sAttr.iFPSMax = FPS_2000_MAX;
        m_sAttr.iFPSMin = FPS_2000_MIN;
        m_sAttr.iSpeedMax = SPEED_312_MAX;
        m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
    }
	else if (m_strLidarModel == X2BF)//speed 6Hz/3Hz
	{
		m_sAttr.dAngleOffsetD = 28.5;
		m_sAttr.dBaseline_mm = 17.92;
		m_sAttr.iFPSMax = FPS_3000_MAX;
		m_sAttr.iFPSMin = FPS_3000_MIN;
		m_sAttr.iSpeedMax = SPEED_360_MAX;
		m_sAttr.iSpeedMin = SPEED_360_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_3000;
		m_sAttr.dCirclePoints = CICRLE_MAX_3000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_3000_NOR;
	}
	else if (m_strLidarModel == X2BZ)
	{

		m_sAttr.dAngleOffsetD = 28.5;
		m_sAttr.dBaseline_mm = 17.92;
		m_sAttr.iFPSMax = FPS_2000_MAX;
		m_sAttr.iFPSMin = FPS_2000_MIN;
		m_sAttr.iSpeedMax = SPEED_312_MAX;
		m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
	}
	else if (m_strLidarModel == X2B2)
	{

		m_sAttr.dAngleOffsetD = 28.5;
		m_sAttr.dBaseline_mm = 17.92;
		m_sAttr.iFPSMax = FPS_2000_MAX;
		m_sAttr.iFPSMin = FPS_2000_MIN;
		m_sAttr.iSpeedMax = SPEED_312_MAX;
		m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
	}
	else if (m_strLidarModel == X2DE)
	{
		m_sAttr.dAngleOffsetD = 28.5;
		m_sAttr.dBaseline_mm = 17.92;
		m_sAttr.iFPSMax = FPS_3000_MAX;
		m_sAttr.iFPSMin = FPS_3000_MIN;
		m_sAttr.iSpeedMax = SPEED_360_MAX;
		m_sAttr.iSpeedMin = SPEED_360_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_3000;
		m_sAttr.dCirclePoints = CICRLE_MAX_3000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_3000_NOR;
	}
	else if (m_strLidarModel == X2MF)//speed 6Hz/3Hz
	{
		m_sAttr.dAngleOffsetD = 28.5;
		m_sAttr.dBaseline_mm = 17.92;
		m_sAttr.iFPSMax = FPS_2000_MAX;
		m_sAttr.iFPSMin = FPS_2000_MIN;
		m_sAttr.iSpeedMax = SPEED_360_MAX;
		m_sAttr.iSpeedMin = SPEED_360_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000_6HZ;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000_6HZ;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
	}
	else if (m_strLidarModel == X2MX)
	{
		m_sAttr.dAngleOffsetD = 28.5;
		m_sAttr.dBaseline_mm = 17.92;
		m_sAttr.iFPSMax = FPS_2000_MAX;
		m_sAttr.iFPSMin = FPS_2000_MIN;
		m_sAttr.iSpeedMax = SPEED_312_MAX;
		m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
	}
	else if (m_strLidarModel == X2MM)
	{
		m_sAttr.dAngleOffsetD = 28.5;
		m_sAttr.dBaseline_mm = 17.92;
		m_sAttr.iFPSMax = FPS_3000_MAX;
		m_sAttr.iFPSMin = FPS_3000_MIN;
		m_sAttr.iSpeedMax = SPEED_360_MAX;
		m_sAttr.iSpeedMin = SPEED_360_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_3000;
		m_sAttr.dCirclePoints = CICRLE_MAX_3000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_3000_NOR;
	}
	else if (m_strLidarModel == D2A)
	{
		m_sAttr.dAngleOffsetD = 28.5;
		m_sAttr.dBaseline_mm = 17;
		m_sAttr.dTheta_d = 0;
		m_sAttr.iFPSMax = FPS_2000_MAX;
		m_sAttr.iFPSMin = FPS_2000_MIN;
		m_sAttr.iSpeedMax = SPEED_312_MAX;
		m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
	}
	else if (m_strLidarModel == D2B)
	{
		m_sAttr.dAngleOffsetD = 28.5;
		m_sAttr.dBaseline_mm = 17;
		m_sAttr.dTheta_d = 0;
		m_sAttr.iFPSMax = FPS_3000_MAX;
		m_sAttr.iFPSMin = FPS_3000_MIN;
		m_sAttr.iSpeedMax = SPEED_360_MAX;
		m_sAttr.iSpeedMin = SPEED_360_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_3000_NOR;
	}
	else if (m_strLidarModel == D2M8)
	{
		m_sAttr.dAngleOffsetD = 28.5;
		m_sAttr.dBaseline_mm = 17;
		m_sAttr.dTheta_d = 0;
		m_sAttr.iFPSMax = FPS_2000_MAX;
		m_sAttr.iFPSMin = FPS_2000_MIN;
		m_sAttr.iSpeedMax = SPEED_312_MAX;
		m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
	}
	else if (m_strLidarModel == D2A8)
	{
		m_sAttr.dAngleOffsetD = 28.5;
		m_sAttr.dBaseline_mm = 17;
		m_sAttr.dTheta_d = 0;
		m_sAttr.iFPSMax = FPS_2000_MAX;
		m_sAttr.iFPSMin = FPS_2000_MIN;
		m_sAttr.iSpeedMax = SPEED_312_MAX;
		m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
	}
	else if (m_strLidarModel == D2B8)
	{
		m_sAttr.dAngleOffsetD = 28.5;
		m_sAttr.dBaseline_mm = 17;
		m_sAttr.dTheta_d = 0;
		m_sAttr.iFPSMax = FPS_3000_MAX;
		m_sAttr.iFPSMin = FPS_3000_MIN;
		m_sAttr.iSpeedMax = SPEED_360_MAX;
		m_sAttr.iSpeedMin = SPEED_360_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_3000;
		m_sAttr.dCirclePoints = CICRLE_MAX_3000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_3000_NOR;
	}
	else if (m_strLidarModel == D2S8)
	{
		m_sAttr.dAngleOffsetD = 28.5;
		m_sAttr.dBaseline_mm = 17;
		m_sAttr.dTheta_d = 0;
		m_sAttr.iFPSMax = FPS_1800_MAX;
		m_sAttr.iFPSMin = FPS_1800_MIN;
		m_sAttr.iSpeedMax = SPEED_300_MAX;
		m_sAttr.iSpeedMin = SPEED_300_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_1800;
		m_sAttr.dCirclePoints = CICRLE_MAX_1800;
		m_sAttr.u64TSStepNs = 1e9 / FPS_1800_NOR;
	}
	else if (m_strLidarModel == D2SA)
	{
		m_sAttr.dAngleOffsetD = 28.5;
		m_sAttr.dBaseline_mm = 17;
		m_sAttr.dTheta_d = 0;
		m_sAttr.iFPSMax = FPS_1800_MAX;
		m_sAttr.iFPSMin = FPS_1800_MIN;
		m_sAttr.iSpeedMax = SPEED_300_MAX;
		m_sAttr.iSpeedMin = SPEED_300_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_1800;
		m_sAttr.dCirclePoints = CICRLE_MAX_1800;
		m_sAttr.u64TSStepNs = 1e9 / FPS_1800_NOR;
	}
	else if (m_strLidarModel == D2P8)
	{
		m_sAttr.dAngleOffsetD = 28.5;
		m_sAttr.dBaseline_mm = 17;
		m_sAttr.dTheta_d = 0;
		m_sAttr.iFPSMax = FPS_2000_MAX;
		m_sAttr.iFPSMin = FPS_2000_MIN;
		m_sAttr.iSpeedMax = SPEED_312_MAX;
		m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
	}
	else if (m_strLidarModel == D2PD)
	{
		m_sAttr.dAngleOffsetD = 28.5;
		m_sAttr.dBaseline_mm = 17;
		m_sAttr.dTheta_d = 0;
		m_sAttr.iFPSMax = FPS_2000_MAX;
		m_sAttr.iFPSMin = FPS_2000_MIN;
		m_sAttr.iSpeedMax = SPEED_312_MAX;
		m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
	}
	else if (m_strLidarModel == D2M1)
	{
		m_sAttr.dAngleOffsetD = 28.5;
		m_sAttr.dBaseline_mm = 17;
		m_sAttr.dTheta_d = 0;
		m_sAttr.iFPSMax = FPS_2000_MAX;
		m_sAttr.iFPSMin = FPS_2000_MIN;
		m_sAttr.iSpeedMax = SPEED_312_MAX;
		m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
	}
	else if (m_strLidarModel == D2AE)
	{
		m_sAttr.dAngleOffsetD = 28.5;
		m_sAttr.dBaseline_mm = 17;
		m_sAttr.dTheta_d = 0;
		m_sAttr.iFPSMax = FPS_2000_MAX;
		m_sAttr.iFPSMin = FPS_2000_MIN;
		m_sAttr.iSpeedMax = SPEED_312_MAX;
		m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
	}
	else if (m_strLidarModel == T3B)
	{
		m_sAttr.dAngleOffsetD = 21;
		m_sAttr.dBaseline_mm = 20;
		m_sAttr.iFPSMax = FPS_TOF_MAX;
		m_sAttr.iFPSMin = FPS_TOF_MIN;
		m_sAttr.iSpeedMax = SPEED_TOF_MAX;
		m_sAttr.iSpeedMin = SPEED_TOF_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_TOF;
		m_sAttr.dCirclePoints = CICRLE_MAX_TOF;
	}
	else if (m_strLidarModel == D3A3)//D2A8
	{
		m_sAttr.dAngleOffsetD = 28.5;
		m_sAttr.dBaseline_mm = 17;
		m_sAttr.dTheta_d = 0;
		m_sAttr.iFPSMax = FPS_2000_MAX;
		m_sAttr.iFPSMin = FPS_2000_MIN;
		m_sAttr.iSpeedMax = SPEED_312_MAX;
		m_sAttr.iSpeedMin = SPEED_312_MIN;
		m_sAttr.dAngleStep = ANGLE_RESOLV_2000;
		m_sAttr.dCirclePoints = CICRLE_MAX_2000;
		m_sAttr.u64TSStepNs = 1e9 / FPS_2000_NOR;
	}
    else
    {

        return false;
    }
    return true;
}
//bGetLoopData 是否输出一圈的数据
BOOL HCLidar::initialize(const char* chLidarModel, int iReadTimeoutMs,  bool bDistQ2,bool bGetLoopData, bool bPollMode)
{
	LOG_INFO("Init sdk\n");
    if(m_bScanning)
    {

		LOG_WARNING( "Had been init.\n");
        setReadCharsError(ERR_SDK_HAD_BEEN_INIT);
        return false;
    }

    if(chLidarModel == NULL)
    {
		LOG_ERROR("lidar mode is null.\n");
        setReadCharsError(ERR_SDK_INIT_PARA);
        return false;
    }

    m_iReadTimeOutms = iReadTimeoutMs;
    if( m_iReadTimeOutms<= 0 || m_iReadTimeOutms >= 50)
    {
        m_iReadTimeOutms = READ_TIMEOUT_MS;
    }
	
    if(!setLidarPara(chLidarModel))
    {
		LOG_ERROR("Lidar mode does not exists.\n");
        setReadCharsError(ERR_MODEL_NOT_EXISTS);
        return false;
    }
	resetParam();

    m_iSDKStatus = SDK_INIT;
	m_bScanning = false;
    m_bGetLoopData = bGetLoopData;
    m_bPollMode = bPollMode;
    m_bDistQ2 = bDistQ2;
    

	LOG_INFO("Init complete\n");


	return true;
}

bool HCLidar::getLidarInfo()
{
	if (m_bHadID)
	{
		return true;
	}
	std::unique_lock<std::mutex> lck(m_mtxInit);
	while (!m_bReady)
		m_cvInit.wait(lck);

	return m_bHadID;
}


void HCLidar::setReadCharsError(int errCode)
{
    switch (errCode)
    {
	case -6:
		m_iLastErrorCode = ERR_SERIAL_READFILE_FAILED;
		break;
    case -5:
        m_iLastErrorCode = ERR_SERIAL_INVALID_HANDLE;
        break;
    case -4:
        m_iLastErrorCode = ERR_SERIAL_SETCOMMTIMEOUTS_FAILED;
        break;
    case -3:
        m_iLastErrorCode = ERR_SERIAL_READFILE_FAILED;
        break;
    case -1:
        m_iLastErrorCode = ERR_SERIAL_READFILE_FAILED;
        break;
    case 0:
        m_iLastErrorCode = ERR_SERIAL_READFILE_ZERO;
        break;
    default:
        m_iLastErrorCode = errCode;
        break;
    }
	int iTemp = m_iLastErrorCode;
	LOG_ERROR("Error code:%d\n", iTemp);

    if(m_funErrorCode)
    {
        m_funErrorCode(m_iLastErrorCode);
    }

	std::lock_guard<std::mutex> lock(m_mtxError);
	m_mapErrorCode[m_iLastErrorCode] = HCHead::getCurrentTimestampUs();
}


void HCLidar::readData(const QByteArray &data)
{
    QByteArray m_data = data;
    uint64_t data_size = data.size();
    uint8_t *dataPtr = reinterpret_cast<uint8_t*>(m_data.data());
    threadParse(dataPtr, data_size);
}

void HCLidar::threadParse(uint8_t *data, uint64_t size)
{
	m_lstBuff.insert(m_lstBuff.end(), data, data+size);
    processMain();

    {
        checkReadPacketData();

        checkChangeSpeed();
    }
}


void HCLidar::checkReadPacketData()
{

   // if(m_sStatistic.u64TSRxPacketFirst == 0)
        //return;


    if(bIntervalOneSecond(m_u64StartMS))
    {
        m_sStatistic.u64CurrentS++;
        m_sStatistic.u64FPS = m_sStatistic.iPacketPerSecond*m_sStatistic.iNumPerPacket;

        m_sStatistic.u64TimeStampS = HCHead::getCurrentTimestampUs()/1e6;
        checkInvalidFPS(m_sStatistic.u64FPS);

		m_sStatistic.dRMS = 0;
		double dSum = std::accumulate(m_lstSpeed.begin(), m_lstSpeed.end(), 0);
		if (m_lstSpeed.size() > 0)
		{
			m_sStatistic.dRMS = dSum / m_lstSpeed.size(); //均值
			m_lstSpeed.clear();
		}
			


        if(m_funSecondInfo)
        {
            m_funSecondInfo(m_sStatistic);
        }
        m_sStatistic.reset();

		m_u64CountS++;
		checkLDSVoltage();
    }
}

void HCLidar::processMain()
{
    if(m_lstBuff.size()==0)
        return;
    while (true)
    {

        if(!processData())
            break;
        if(m_lstBuff.size()==0)
            break;

    }
    //printf("Info: buff size = %d\n",m_lstBuff.size());
}


void HCLidar::sendGetFactoryInfoSignal(bool bGetFact)
{
    if(!bGetFact)
    {
        setReadCharsError(ERR_NOT_ID);
    }

    m_bHadFact = bGetFact;
    checkHadInitSuccess(false);
}

void HCLidar::checkHadInitSuccess(bool bTimeout)
{
    if(bTimeout)
    {
        m_iSDKStatus = SDK_ID_TIMEOUT;
        std::unique_lock <std::mutex> lck(m_mtxInit);
        m_bReady = true;
        m_cvInit.notify_all();
    }
    else
    {
        if(m_bHadID && m_bHadFact)
        {
            m_iSDKStatus = SDK_WORKING;
            std::unique_lock <std::mutex> lck(m_mtxInit);
            m_bReady = true;
            m_cvInit.notify_all();
        }
		else// if (m_bHadID && !m_bHadFact)
		{
			m_iSDKStatus = SDK_ID_TIMEOUT;
			std::unique_lock <std::mutex> lck(m_mtxInit);
			m_bReady = true;
			m_cvInit.notify_all();
		}
    }
}

bool HCLidar::processData()
{
    if(m_lstBuff.size()<=2)
        return false;

	if (m_sStatistic.iErrorCountContinue >= NUMBER_CONTINUE_ERROR_PACKET)
	{
		m_sStatistic.iErrorCountContinue = 0;
		if (m_lstBuff.size() > 3)
			HCHead::eraseBuff(m_lstBuff, m_lstBuff.size() - 1);
		else
			m_lstBuff.clear();
		LOG_ERROR("Continuous Rx error packet:%d!\n", NUMBER_CONTINUE_ERROR_PACKET);
		setReadCharsError(ERR_RX_CONTINUE);
		return false;
	}
    int iIndex = -1;
    int iMsgID = 0;
    for(int i=0;i<m_lstBuff.size()-1;i++)
    {
        
        if(m_iSDKStatus == SDK_INIT || (m_bFactoryMode && m_iSDKStatus == SDK_DISCONNECT))
        {
			if (m_strLidarModel == X1S)
			{
				if (!m_bHadID && !m_bGetIDTimeOut)
				{
					if ((UCHAR)m_lstBuff.at(i) == 0xAA)//ID
					{
						iMsgID = MSG_ID;
						iIndex = i;
						break;
					}
				}
			}
			else
			{
				if (!m_bHadID && !m_bGetIDTimeOut)
				{
					if ((UCHAR)m_lstBuff.at(i) == 0x5A && (UCHAR)m_lstBuff.at(i + 1) == 0xA5)//ID
					{
						iMsgID = MSG_ID;
						iIndex = i;
						break;
					}
					
				}
				if (!m_bHadFact && !m_bGetFactTimeOut)
				{
					if ((UCHAR)m_lstBuff.at(i) == 0x5A && (UCHAR)m_lstBuff.at(i + 1) == 0xAA)//SN
					{
						iMsgID = MSG_NEW_SN;
						iIndex = i;
						break;
					}

					if ((UCHAR)m_lstBuff.at(i) == 0xA5 && (UCHAR)m_lstBuff.at(i + 1) == 0x5A)//Two CMD
					{
						iMsgID = MSG_CMD;
						iIndex = i;
						break;
					}
				}
			}
                
        }

        if((UCHAR)m_lstBuff.at(i) == 0x55 && (UCHAR)m_lstBuff.at(i+1) == 0xAA)//point cloud
        {
            iMsgID = MSG_POINTCLOUD;
            iIndex = i;
            break;
        }
        
    }
    if(iIndex < 0)
    {
        //LOG_INFO("HCSDK Error: rx data not mes header\n" );
        m_lstBuff.clear();
		
		checkFindPackHeader();
        return false;
    }
	else
	{
		m_u64StartTimeFindPackHeader = 0;
	}

    if(iIndex>0)
    {
        HCHead::eraseBuff(m_lstBuff,iIndex);
		//LOG_INFO("HCSDK Error: find mes header,buff size=%d\n" , m_lstBuff.size());
    }


    switch (iMsgID) {
    case MSG_ID:
        if (getDevID(m_lstBuff))
        {

            return true;
        }
        else
            return false;
        //break;
    case MSG_CMD:
            return getStartInfo(m_lstBuff);
	case MSG_NEW_SN:
        if (getNewSNInfo(m_lstBuff))
        {
            SnFun(getLidarID());
            return true;
        }
        else
            return false;
       // break;
    case MSG_POINTCLOUD:
		return getPointCloud(m_lstBuff);

    }

    return false;

}

void HCLidar::checkFindPackHeader()
{
	if (m_bInitTimeout)
	{
		if (m_u64StartTimeFindPackHeader > 0)
		{
			UINT64 u64Temp = HCHead::getCurrentTimestampUs();
			UINT64 u64Int = u64Temp - m_u64StartTimeFindPackHeader;

			if (u64Int >= 100000) //100ms
			{
				m_u64StartTimeFindPackHeader = 0;
				setReadCharsError(ERR_FIND_HEAD_TIMEOUT);
			}
		}
		else
		{
			m_u64StartTimeFindPackHeader = HCHead::getCurrentTimestampUs();
		}
	}
	else
	{
		m_u64StartTimeFindPackHeader = 0;
	}
}

bool HCLidar::calIDX2(char* ch,int iLen)
{
    UINT sum = 0;
    const UINT id_len = iLen-1;
    for (UINT i = 0; i < id_len; i++)
    {
       sum += (UCHAR)ch[i];
    }

    const UINT sum_check = (UCHAR)ch[id_len];
    unsigned char temp_sum = sum & 0xff;
    return (sum_check == temp_sum);
}

bool HCLidar::calIDX1(char* ch,int iLen)
{
    UINT sum = 0;
    const UINT id_len = iLen-2;
    for (UINT i = 0; i < id_len; i++)
    {
        sum += (UCHAR)ch[i];
    }

    const UINT sum_check = ((UCHAR)ch[id_len + 1] << 8) | (UCHAR)ch[id_len];
    return (sum_check == sum);
}

bool HCLidar::getDevID(std::vector<UCHAR>& lstBuff)
{
	char chTemp[128] = { 0 };
	if (m_strLidarModel == "X1S")//rock
	{
		unLidarInfo lds;

		int iLen = sizeof(unLidarInfo) + 1;
		if (lstBuff.size() < iLen)
		{
			return false;
		}
		bool bOK = rockCheckLDSInfo((uint8_t*)lstBuff.data() + 1, lds);
		if (bOK)
		{
			memset(chTemp, 0, 128);
			sprintf(chTemp, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				(lds.sAttr.sUid.UID_0 & 0x00ff),
				(lds.sAttr.sUid.UID_0 & 0xff00) >> 8,
				(lds.sAttr.sUid.UID_1 & 0x00ff),
				(lds.sAttr.sUid.UID_1 & 0xff00) >> 8,
				(lds.sAttr.sUid.UID_2 & 0x00ff),
				(lds.sAttr.sUid.UID_2 & 0xff00) >> 8,
				(lds.sAttr.sUid.UID_3 & 0x00ff),
				(lds.sAttr.sUid.UID_3 & 0xff00) >> 8,
				(lds.sAttr.sUid.UID_4 & 0x00ff),
				(lds.sAttr.sUid.UID_4 & 0xff00) >> 8,
				(lds.sAttr.sUid.UID_5 & 0x00ff),
				(lds.sAttr.sUid.UID_5 & 0xff00) >> 8,
				(lds.sAttr.sUid.UID_6 & 0x00ff),
				(lds.sAttr.sUid.UID_6 & 0xff00) >> 8,
				(lds.sAttr.sUid.UID_7 & 0x00ff),
				(lds.sAttr.sUid.UID_7 & 0xff00) >> 8,
				(lds.sAttr.sUid.UID_8 & 0x00ff),
				(lds.sAttr.sUid.UID_8 & 0xff00) >> 8);

			m_strDevID = chTemp;


			LOG_INFO("Get ID ok!\n");
			//sendGetIDInfoSignal(true);
			m_bHadID = true;
			
			
			UINT16 u16Temp = ~lds.sAttr.u16AngleOffset + 1;//原码
			float fZeroAngle = u16Temp *0.01;
	
			memset(chTemp, 0, 128);
			sprintf(chTemp, "ZeroAngle=%0.2f\n",fZeroAngle);
			LOG_INFO(chTemp);


			memset(chTemp, 0, 128);
			sprintf(chTemp, "00.00.%02X.%02X",
				(lds.sAttr.u16Version & 0xFF00) >> 8, lds.sAttr.u16Version & 0x00FF);

			m_strFirmwareVer = chTemp;

			sendGetFactoryInfoSignal(true);

		}

		HCHead::eraseBuff(lstBuff, iLen);
		return bOK;

	}

    
    if(m_bX2ID)//X2 series
    {
        int iLen = sizeof(tsIDX2);
        if(lstBuff.size() < iLen)
            return false;


        tsIDX2  sID;
        memcpy(&sID, lstBuff.data(), iLen);
        //memcpy(&m_unDevID.u32ID, sID.u8ID, ID_LEN);

        if(calIDX2((char*)lstBuff.data(),iLen))
        {

			sprintf(chTemp, "%02X%02X%02X%02X%02X%02X%02X",
				sID.u8Ver[0], sID.u8Ver[1], sID.u8Ver[2],
				sID.u8ID[0], sID.u8ID[1], sID.u8ID[2], sID.u8ID[3]);

            m_strDevID = chTemp;


            memset(chTemp,0,128);
            sprintf(chTemp, "00.%02X.%02X.%02X",
                                sID.u8Ver[0], sID.u8Ver[1], sID.u8Ver[2]);

            m_strFirmwareVer = chTemp;


			LOG_INFO("Get ID ok!\n");
            //sendGetIDInfoSignal(true);
			m_bHadID = true;

        }
        else
        {
			if (m_strLidarModel == X2N)
				m_strDevID = std::string(DEFAULT_ID);
			else
				m_strDevID = "";


			LOG_ERROR("ID cal error!\n" );


        }
        //lstBuff.erase(lstBuff.begin(), lstBuff.begin() + iLen);
        HCHead::eraseBuff(lstBuff,iLen);

        return true;
    }
    else//X1
    {
        int iLen = sizeof(tsIDX1);
        if(lstBuff.size() < iLen)
            return false;


        tsIDX1  sID;
        memcpy(&sID, lstBuff.data(), iLen);
        //memcpy(&m_unDevID.u32ID, sID.u8ID, ID_LEN);

        if(calIDX1((char*)lstBuff.data(),iLen))
        {

			sprintf(chTemp, "%02X%02X%02X%02X",
				sID.u8ID[3], sID.u8ID[2], sID.u8ID[1], sID.u8ID[0]);


            m_strDevID = chTemp;
            //m_bHadID = true;
			LOG_INFO("Get ID ok!\n" );
            //sendGetIDInfoSignal(true);
			m_bHadID = true;

        }
        else
        {
            m_strDevID = std::string(DEFAULT_ID);

			LOG_ERROR("ID cal error!\n");


        }
        //lstBuff.erase(lstBuff.begin(), lstBuff.begin() + iLen);
        HCHead::eraseBuff(lstBuff,iLen);

        return true;
    }

}

bool HCLidar::calStartInfo(char* ch,int iLen)
{
    UINT sum = 0;

    for (UINT i = 0; i < iLen; i++)
    {
        if(i==4 || i==5)
        {

        }
        else
            sum += (UCHAR)ch[i];
    }

    const UINT sum_check = ((UCHAR)ch[5] << 8) | (UCHAR)ch[4];
    return (sum_check == sum);
}

bool HCLidar::getNewSNInfo(std::vector<UCHAR>& lstBuff)
{

	LOG_INFO("getNewSNInfo!\n");

	if (lstBuff.size() < sizeof(tsSDKSN))
		return false;

	tsSDKSN sNewInfo;
	memcpy(&sNewInfo, lstBuff.data(), sizeof(tsSDKSN));

	if (calStartInfo((char*)lstBuff.data(), sizeof(tsSDKSN)))
	{
		HCHead::eraseBuff(lstBuff, sizeof(tsSDKSN));

		char chTemp[128] = { 0 };
		sprintf(chTemp, "%c%c",
			sNewInfo.u8FacInfo[0], sNewInfo.u8FacInfo[1]);
		m_strFactoryInfo = chTemp;

		memset(chTemp, 0, 128);
		sprintf(chTemp, "%c%d%c%c", sNewInfo.u8FacInfo[2], sNewInfo.u8FacInfo[3], sNewInfo.u8FacInfo[4], sNewInfo.u8FacInfo[5]);
		std::string strModel = chTemp;
		if (strModel != m_strLidarModel)
		{
			LOG_WARNING("Lidar model error init:%s,device:%s\n", (char*)m_strLidarModel.c_str(), (char*)strModel.c_str());
			//setReadCharsError(ERR_DEV_MODEL);
			setLidarPara(chTemp);
		}

		memset(chTemp, 0, 128);

		sprintf(chTemp, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
			sNewInfo.u8SN[0],
			sNewInfo.u8SN[1],
			sNewInfo.u8SN[2],
			sNewInfo.u8SN[3],
			sNewInfo.u8SN[4],
			sNewInfo.u8SN[5],
			sNewInfo.u8SN[6],
			sNewInfo.u8SN[7],
			sNewInfo.u8SN[8],
			sNewInfo.u8SN[9],
			sNewInfo.u8SN[10],
			sNewInfo.u8SN[11],
			sNewInfo.u8SN[12],
			sNewInfo.u8SN[13],
			sNewInfo.u8SN[14],
			sNewInfo.u8SN[15],
			sNewInfo.u8SN[16],
			sNewInfo.u8SN[17], 
			sNewInfo.u8SN[18], 
			sNewInfo.u8SN[19] );
		//sprintf(chTemp, "%s", sNewInfo.u8SN);

		//m_strDevID = chTemp;

		LOG_INFO("Get SN:%s\n", chTemp);


		memset(chTemp, 0, 128);
		sprintf(chTemp, "00.00.%02X.%02X",
			sNewInfo.u8CalVer[0], sNewInfo.u8CalVer[1]);
		m_strFirmwareVer = chTemp;

		sprintf(chTemp, "%02X.%02X.%02X.%02X",
			sNewInfo.u8HardVer[0], sNewInfo.u8HardVer[1], sNewInfo.u8HardVer[2], sNewInfo.u8HardVer[3]);
		m_strHardwareVer = chTemp;
		
		m_bHadID = true;
		//sendGetIDInfoSignal(true);

		sendGetFactoryInfoSignal(true);
		LOG_INFO("New lidar factory info:%s,Hardware ver:%s\n", (char*)m_strFactoryInfo.c_str(), (char*)m_strHardwareVer.c_str());
		return true;
	}
	else
	{
		HCHead::eraseBuff(lstBuff, sizeof(tsSDKSN));

		LOG_ERROR("New lidar factory info cal error\n");
		setReadCharsError(ERR_START_INFO);
		return true;

	}
    return false;
}
bool HCLidar::getStartInfo(std::vector<UCHAR>& lstBuff)
{

	LOG_INFO("getStartInfo!\n");

	int iLen = sizeof(tsCmdInfo);
	int iMin = iLen < lstBuff.size() ? iLen : lstBuff.size();
	if (lstBuff.size() < sizeof(tsCmdStart))
		return false;

	tsCmdInfo  sCmd;
	memcpy(&sCmd, lstBuff.data(), iMin);
	if (sCmd.u16Len == 0)
	{

		HCHead::eraseBuff(lstBuff, sizeof(tsCmdStart));

		LOG_ERROR("lidar start message\n");
		setReadCharsError(ERR_START_INFO);
		return true;
	}

	if (sCmd.u16Len == 20)//old
	{
		if (calStartInfo((char*)lstBuff.data(), iLen))
		{
			HCHead::eraseBuff(lstBuff, sizeof(tsCmdStart));

			m_strFactoryInfo = std::string((char*)sCmd.u8FacInfo);


			sendGetFactoryInfoSignal(true);
			LOG_INFO("lidar factory info:%s\n", (char*)sCmd.u8FacInfo);
			return true;
		}
		else
		{
			HCHead::eraseBuff(lstBuff, sizeof(tsCmdStart));

			LOG_ERROR("lidar start message cal error\n");
			setReadCharsError(ERR_START_INFO);
			return true;

		}
	}
	else if (sCmd.u16Len == 25)//NEW
	{
		if (lstBuff.size() < sizeof(tsSDKIDNew))
			return false;

		tsSDKIDNew sNewInfo;
		memcpy(&sNewInfo, lstBuff.data(), sizeof(tsSDKIDNew));

		if (calStartInfo((char*)lstBuff.data(), sizeof(tsSDKIDNew)))
		{
			HCHead::eraseBuff(lstBuff, sizeof(tsSDKIDNew));

			char chTemp[128] = { 0 };
			sprintf(chTemp, "%c%c",
				sNewInfo.u8FacInfo[0], sNewInfo.u8FacInfo[1]);
			m_strFactoryInfo = chTemp;

			memset(chTemp, 0, 128);
			sprintf(chTemp, "%c%d%c", sNewInfo.u8FacInfo[2], sNewInfo.u8FacInfo[3], sNewInfo.u8FacInfo[4]);
			std::string strModel = chTemp;
			if (strModel != m_strLidarModel)
			{
				LOG_WARNING("Lidar model error init:%s,device:%s\n", (char*)m_strLidarModel.c_str(), (char*)strModel.c_str());
				//setReadCharsError(ERR_DEV_MODEL);
				setLidarPara(chTemp);
			}

			memset(chTemp, 0, 128);

			if (m_strLidarModel == X2N)
			{
				sprintf(chTemp, "00000000000000000000%02X%02X%02X%02X%02X%02X%02X",
					sNewInfo.u8FacInfo[3], sNewInfo.u8FacInfo[4], sNewInfo.u8FacInfo[5],
					sNewInfo.u8ID[0], sNewInfo.u8ID[1], sNewInfo.u8ID[2], sNewInfo.u8ID[3]);
			}
			else
			{
				sprintf(chTemp, "%02X%02X%02X%02X%02X%02X%02X",
					sNewInfo.u8FacInfo[3], sNewInfo.u8FacInfo[4], sNewInfo.u8FacInfo[5],
					sNewInfo.u8ID[0], sNewInfo.u8ID[1], sNewInfo.u8ID[2], sNewInfo.u8ID[3]);
			}


			m_strDevID = chTemp;

			LOG_INFO("Get ID ok ID:%s\n", chTemp);


			memset(chTemp, 0, 128);
			sprintf(chTemp, "00.00.%02X.%02X",
				sNewInfo.u8CalVer[0], sNewInfo.u8CalVer[1]);

			m_strFirmwareVer = chTemp;


			memset(chTemp, 0, 128);
			sprintf(chTemp, "00.%02X.%02X.%02X",
				sNewInfo.u8HardVer[0], sNewInfo.u8HardVer[1], sNewInfo.u8HardVer[2]);

			m_strHardwareVer = chTemp;

			m_bHadID = true;
			//sendGetIDInfoSignal(true);

			sendGetFactoryInfoSignal(true);
			LOG_INFO("New lidar factory info:%s,Hardware ver:%s\n", (char*)m_strFactoryInfo.c_str(), (char*)m_strHardwareVer.c_str());
			return true;
		}
		else
		{
			HCHead::eraseBuff(lstBuff, sizeof(tsSDKIDNew));

			LOG_ERROR("New lidar factory info cal error\n");
			setReadCharsError(ERR_START_INFO);
			return true;

		}
	}
	else
	{
		HCHead::eraseBuff(lstBuff, sizeof(tsCmdStart));

		LOG_ERROR("lidar start message\n");
		setReadCharsError(ERR_START_INFO);
		return true;
	}
	return false;
}
bool HCLidar::calMCUFrame(char* ch,int iLen)
{
    UINT iSUM = 0;
    for (UINT i = 0; i < iLen-1; i++)
    {
        iSUM += (UCHAR)ch[i];
    }

    UCHAR u8Sum = (UCHAR)iSUM;
    return (u8Sum == (UCHAR)ch[iLen-1]);
}

bool HCLidar::getMCUCmd(std::vector<UCHAR>& lstBuff)
{

    tsBlockMessage  sBlockMessage;
    memcpy(&sBlockMessage, lstBuff.data(), sizeof(tsBlockMessage));

    int iPackLen = sBlockMessage.u8Len; // u8Num is length
    if(iPackLen <= lstBuff.size())//have a good packet
    {
        if(calMCUFrame((char *)lstBuff.data(),iPackLen))
        {
            if(sBlockMessage.u8MsgID == 0x45) //block message for motor
            {
                if(sBlockMessage.u8Code == 0x01)
                {
					LOG_INFO("MCU message <Motor blocked>\n" );

                    /*if(m_iLastErrorCode != ERR_MOTOR_BLOCKED)
                    {
                        setReadCharsError(ERR_MOTOR_BLOCKED);


                    }*/
                    checkSharkBlocked();
                }
                else
                {

					LOG_INFO("MCU message <Lidar reboot>\n");
                    /*if(m_iLastErrorCode != ERR_REBOOT_LIDAR)
                        setReadCharsError(ERR_REBOOT_LIDAR);*/
                }
            }
        }
        HCHead::eraseBuff(lstBuff,iPackLen);
        return true;
    }
    else
    {
        return false;
    }
}

bool HCLidar::getPointCloud(std::vector<UCHAR>& lstBuff)
{

	//printf("Info: getPointCloud!\n");

	int iLen = sizeof(tsPointCloudHead);
	int iMin = iLen < lstBuff.size() ? iLen : lstBuff.size();
	if (lstBuff.size() < iLen)
		return false;

	tsPointCloudHead  sPointCloudHead;
	memcpy(&sPointCloudHead, lstBuff.data(), iMin);

	bool bBlockMessage = false;
	int iPointBytes = 3;
	if (sPointCloudHead.u8Info == 0x02)
	{
		m_sStatistic.iGrayBytes = 0;
		iPointBytes = 2;
	}
	else if (sPointCloudHead.u8Info == 0x07)
	{
		m_sStatistic.iGrayBytes = 2;
		iPointBytes = 4;
	}
	else if (sPointCloudHead.u8Info == 0x80)//MCU message
	{
		bBlockMessage = true;
	}
	else
	{
		m_sStatistic.iGrayBytes = 1;
		iPointBytes = 3;
	}

	if (bBlockMessage)
	{
		return getMCUCmd(lstBuff);
	}

	int iPackLen = sPointCloudHead.u8Num * iPointBytes + sizeof(tsPointCloudHead) + sizeof(tsPointCloudTail);

	if (iPackLen <= lstBuff.size())//have a good packet
	{
		/*if (m_sStatistic.u64TSRxPacketFirst == 0)
		{

			m_sStatistic.u64TSRxPacketFirst = HCHead::getCurrentTimestampUs();
			m_u64StartMS = m_sStatistic.u64TSRxPacketFirst;

		}*/
		if (checkDataCal(lstBuff, iPackLen))//good packet
		{

			m_sStatistic.iErrorCountContinue=0;
			m_sStatistic.u64RxPacketCount++;
			m_sStatistic.iNumPerPacket = sPointCloudHead.u8Num;


			if (m_bPollMode)
			{
				std::lock_guard<std::mutex> lock(m_mtxData);
				parserRangeEX(m_resultRange, (char *)lstBuff.data(), iPackLen, sPointCloudHead.u8Num, iPointBytes);
				pollModePointCloud();
			}
			else//call back mode
			{
				parserRangeEX(m_resultRange, (char *)lstBuff.data(), iPackLen, sPointCloudHead.u8Num, iPointBytes);
				callBackFunPointCloud();
				
			}

			//LOG_INFO("lidar point cal ok! Point cloud buff size=%d\n" ,m_resultRange.size() );
		}
		else
		{
			m_sStatistic.iErrorCountContinue++;
			LOG_ERROR("lidar point cloud error!\n");
			m_sStatistic.u64ErrorPacketCount++;
			setReadCharsError(ERR_CHECKDATA_INVALID);

		}
		m_sStatistic.iPacketPerSecond++;

		HCHead::eraseBuff(lstBuff, iPackLen);

		return true;
	}
	return false;

}


bool HCLidar::parserRangeEX(LstPointCloud &resultRange, const char * chBuff, int iIndex, int in_numData, int iPointSize)
{

	UINT64 epochTime = HCHead::getCurrentTimestampNs();

	m_u64TSStart = epochTime;

	UINT64 u64Inter = (m_u64TSStart - m_u64TSEnd)/ (in_numData - 1);
	//LOG_INFO("TS:%lld , %lld\n", u64Inter, m_sAttr.u64TSStepNs);

	const int data_size = iPointSize;//const int data_size = 3;
	int id_start = 6;//int id_start = 2;
	double FA = ((UCHAR)chBuff[id_start + 1] - 0xA0 + (UCHAR)chBuff[id_start] / 256.0) * 4;
	int id_LA_start = id_start + in_numData * data_size + 2;
	double LA = ((UCHAR)chBuff[id_LA_start + 1] - 0xA0 + (UCHAR)chBuff[id_LA_start] / 256.0) * 4;
	if (LA < FA) { LA += 360; }
	double dAngle = (LA - FA) / (in_numData - 1);        // angle info for each sampling
	UCHAR *data = new UCHAR[data_size];//unsigned char data[3];
	int pre_bytes = 8;          // 4 bytes before sampling data in each data package
	unsigned int speed = ((UCHAR)chBuff[5] << 8 | (UCHAR)chBuff[4]) / 64;
	m_lstSpeed.push_back(speed);


	//LOG_INFO("Start=%0.4f,end=%0.4f\n", FA, LA);

	double angle_cur = 0;
	const double angle_offset = m_sAttr.dAngleOffsetD;
	for (int i = 0; i < in_numData; ++i)
	{
		//double Beforecompensation = FA + dAngle * i;
		//double angle_cur = FA + dAngle * i + angle_offset;

		if (m_strLidarModel == "T3B")
			angle_cur = FA + dAngle * i;
		else
			angle_cur = FA + dAngle * i + angle_offset;

		//double Aftercompensation = angle_cur;
		if (angle_cur > 360)
		{
			angle_cur -= 360;
		}


		tsPointCloud sData;
		sData.u64TimeStampNs = epochTime - u64Inter *(in_numData-i);
		//LOG_INFO("TS:%lld\n", sData.u64TimeStampNs);

		memcpy(data, (UCHAR*)chBuff + pre_bytes + i * data_size, sizeof(UCHAR) * data_size);
		if (((data[1] >> 7) & 0x01) == 1)
			sData.bValid = false;//(data[1] >> 7) & 0x01;
		sData.u16DistRaw = ((data[1] & 0x003F) << 8) | data[0];
		sData.u16Dist = sData.u16DistRaw;
		if (data_size == 2)
			sData.u16Gray = 0;
		else if (data_size == 3)
			sData.u16Gray = data[2] & 0xff;
		else if (data_size == 4)
		{
			sData.bGrayTwoByte = true;
			sData.u16Gray = ((data[3] & 0x00ff) << 8) | data[2];//data[2] + ((data[3]&0x00ff) << 8);
		}
		else
			sData.u16Gray = 0;
		sData.u16Speed = speed;
		sData.dAngleRaw = angle_cur;
		sData.dAngle = sData.dAngleRaw;
		sData.dAngleDisp = sData.dAngle;
		// Compensate angle & dist
		if (0 == sData.u16Dist)
		{
			m_sStatistic.iInvalid++;
			sData.bValid = false;        // invalid point

			compensate(sData.dAngle, sData.u16Dist, m_sAttr.dTheta_d, m_sAttr.dBaseline_mm);

		}
		else if (m_bCompensate)
		{
			m_sStatistic.iValid++;
			if (m_strLidarModel == "T3B")
			{
			}
			else
			{
				compensate(sData.dAngle, sData.u16Dist, m_sAttr.dTheta_d, m_sAttr.dBaseline_mm);
			}
			sData.dAngleDisp = sData.dAngle;
		}

		m_sStatistic.dRMS = speed;
		resultRange.push_back(sData);

		//std::cout << "Info: Angle=" << sData.dAngle  << ",AngleRaw=" << sData.dAngleRaw << ",Dist=" << sData.u16Dist << std::endl;
		checkSharkInvalidPoints(sData);
	}
	if (data)
	{
		delete[] data;
		data = NULL;
	}

	checkInvalidLowSpeed(speed);
	checkInvalidHighSpeed(speed);
	checkEncoderError(speed);

	m_u64TSEnd = HCHead::getCurrentTimestampNs();

	return true;
}
void HCLidar::compensate(double &angle, UINT16 &dist, const double theta_d, const double baseline_mm)
{
    if (0 != dist)
    {
        const double theta = theta_d / 180. * PI_HC;
        const double beta = atan(dist / baseline_mm);
        double angle_correct = PI_HC / 2. - (theta + beta);
        angle = angle + angle_correct / PI_HC * 180;
        //angle = angle + 80 - atan(dist / baseline_mm)/ PI_HC * 180;
        if (angle > 360)
        {
            angle -= 360;
        }
        else if (angle < 0)
        {
            angle += 360;
        }

        dist = sqrt(baseline_mm * baseline_mm + dist * dist);
    }
	else
	{
		double dTemp = atan(999999999 / baseline_mm) / PI_HC * 180;
		angle = angle + 80 - dTemp;
		if (angle > 360)
		{
			angle -= 360;
		}
		else if (angle < 0)
		{
			angle += 360;
		}
	}
}

bool HCLidar::checkDataCalTof(std::vector<UCHAR>& lstBuff, int iIndex)
{
	int sum = 0;
	for (int i = 0; i < iIndex - 2; ++i)
	{
		if (i % 2 == 0)
		{
			sum = sum + lstBuff[i];
		}
		else
		{
			sum = sum + (lstBuff[i] << 8);
		}
	}
	unsigned char low = sum;
	unsigned char high = (sum >> 8);
	if (low != lstBuff[iIndex - 2] || high != lstBuff[iIndex - 1])
	{
		//printf("low:%02x %02x, hight:%02x, %02x\n", low, lstBuff[iIndex - 2], high, lstBuff[iIndex - 1]);
		//printf("MSG:");
		//for (int i = 0; i < iIndex; ++i)
		//{
			//printf("%02x  ", lstBuff[i]);
		//}

		//printf("\n");
		return false;
	}
	return true;
}

bool HCLidar::checkDataCal(std::vector<UCHAR>& lstBuff, int iIndex)
{
	const int iB = iIndex - 2;
	std::vector<int> temp(iB / 2, 0);
	//temp[0] = 0x55 + (0xAA << 8);
	for (int i = 0; i < temp.size(); i++)
	{
		temp[i] = lstBuff.at(2 * i) + (lstBuff.at(2 * i + 1) << 8);
	}

	int chk32 = 0;
	for (int i = 0; i < temp.size(); i++)
	{
		chk32 = (chk32 << 1) + temp[i];
	}
	
	UINT16 checksum_target = (chk32 & 0x7FFF) + (chk32 >> 15);
	checksum_target = checksum_target & 0x7FFF;
	UINT16 checksum_cur = lstBuff.at(iIndex - 2) + (lstBuff.at(iIndex - 1) << 8);
	bool is_equal = (checksum_target == checksum_cur);
	return is_equal;
}
bool HCLidar::getRxPointClouds(LstPointCloud& lstG)
{
	if (m_strLidarModel == X2N)
	{
		if (!m_bPollMode)
		{
			setReadCharsError(ERR_POLL_MODE);
			return false;
		}
		if (lstG.size() > 0)
		{
			LstPointCloud tmp;
			tmp.swap(lstG);
		}
		int iError = m_iLastErrorCode;

		bool bOK = true;
		switch (iError)
		{
		case ERR_SHARK_MOTOR_BLOCKED:

		case ERR_SHARK_INVALID_POINTS:

		case ERR_LIDAR_SPEED_LOW:

		case ERR_LIDAR_SPEED_HIGH:

		case ERR_DISCONNECTED:
		case ERR_LIDAR_FPS_INVALID:
			bOK = false;

			break;

		default:
			break;
		}

		if (bOK)
		{
			std::lock_guard<std::mutex> lock(m_mtxData);
			if (m_resultRange.size() > 0)
			{
				lstG.swap(m_resultRange);
				std::stable_sort(lstG.begin(), lstG.end(), newComparator);
			}
		}
		else
		{
			std::lock_guard<std::mutex> lock(m_mtxData);
			if (m_resultRange.size() > 0)
			{
				LstPointCloud tmp;
				m_resultRange.swap(tmp);
			}
		}

		return bOK;
	}
	else
	{
		if (!m_bPollMode)
		{
			setReadCharsError(ERR_POLL_MODE);
			return false;
		}
		if (lstG.size() > 0)
		{
			LstPointCloud tmp;
			tmp.swap(lstG);
		}
		std::lock_guard<std::mutex> lock(m_mtxData);
		if (m_bCircle)
		{
			if (m_Circles.size() > 0)
			{
                lstG = *m_Circles.begin();

				//localDensityFilter(tmp, lstG);
                // filter_inf_.filter_type = FullScanFilter::FS_Smooth;
    //     		scan_filter_.filter(tmp, filter_inf_, lstG);
				// lstG.swap(tmp);
				m_Circles.erase(m_Circles.begin());
			}

		}
		else
		{
			if (m_resultRange.size() > 0)
			{
				lstG.swap(m_resultRange);
			}
		}

		return true;
	}
}


void HCLidar::checkInvalidFPS(int iFPS)
{
	if (m_sStatistic.u64CurrentS < 2)
		return;
    if (iFPS < m_sAttr.iFPSMin || iFPS > m_sAttr.iFPSMax)
    {
        ++m_iInvalidFPSSecond;
		int iTemp = m_sSDKPara.iFPSContinueMS / 1000;
        if (m_iInvalidFPSSecond > iTemp)
        {
			LOG_ERROR("FPS:%d \n" , iFPS );
            setReadCharsError(ERR_LIDAR_FPS_INVALID);

            m_iInvalidFPSSecond = 0;
        }
    }
    else
    {
        m_iInvalidFPSSecond = 0;
    }

	if (m_u64CountS>= SENSOR_ERROR_SECOND && (iFPS == 0 || m_iLastErrorCode == ERR_LIDAR_FPS_INVALID))
	{
		if (!getErrorCode(ERR_MOTOR_BLOCKED, SENSOR_ERROR_TIME_MS))
			setReadCharsError(ERR_LIDAR_SENSOR);
	}
}


void HCLidar::checkInvalidLowSpeed(UINT16 u16Speed)
{
	if (m_sStatistic.u64CurrentS < 2)
		return;
    if (u16Speed < m_sAttr.iSpeedMin)
    {
        if (m_u64StartTimeLowSpeed != 0)
        {
            UINT64 endTime = HCHead::getCurrentTimestampUs();
            if ((endTime - m_u64StartTimeLowSpeed) >= m_sSDKPara.iSpeedContinueMS*1000)
            {
                setReadCharsError(ERR_LIDAR_SPEED_LOW);
                m_u64StartTimeLowSpeed = 0;
            }
        }
        else
        {
            m_u64StartTimeLowSpeed = HCHead::getCurrentTimestampUs();
        }
    }
    else
    {
        m_u64StartTimeLowSpeed = 0;
    }
}

void HCLidar::checkInvalidHighSpeed(UINT16 u16Speed)
{
	if (m_sStatistic.u64CurrentS < 2)
		return;
    if (u16Speed > m_sAttr.iSpeedMax)
    {
        if (m_u64StartTimeHighSpeed != 0)
        {
            int64_t endTime = HCHead::getCurrentTimestampUs();
            if ((endTime - m_u64StartTimeHighSpeed) >= m_sSDKPara.iSpeedContinueMS*1000)
            {
                setReadCharsError( ERR_LIDAR_SPEED_HIGH);
                m_u64StartTimeHighSpeed = 0;
            }
        }
        else
        {
            m_u64StartTimeHighSpeed = HCHead::getCurrentTimestampUs();
        }
    }
    else
    {
        m_u64StartTimeHighSpeed = 0;
    }
}

void HCLidar::checkEncoderError(UINT16 u16Speed)
{
	if (u16Speed == 0 || u16Speed == 1023)
	{
		//LOG_INFO("Speed error=%d\n", u16Speed);
		if (m_u64StartTimeSpeed != 0)
		{
			UINT64 endTime = HCHead::getCurrentTimestampUs();
			if ((u16Speed != m_u16Speed) && (endTime - m_u64StartTimeSpeed) <= ENCODER_ERROR_TIME_MS*1000)
			{
				if (m_sStatistic.u64CurrentS > ENCODER_ERROR_SECOND)
				{
					if(!getErrorCode(ERR_MOTOR_BLOCKED, ENCODER_ERROR_TIME_MS))
						setReadCharsError(ERR_LIDAR_ENCODER);
				}
					
				m_u64StartTimeSpeed = 0;
			}
		}
		else
		{
			m_u64StartTimeSpeed = HCHead::getCurrentTimestampUs();
		}

		
	}
	else
	{
		m_u64StartTimeSpeed = 0;
	}
	m_u16Speed = u16Speed;
}

void HCLidar::checkLDSVoltage()
{
	if (m_u64CountS >= LDS_VOLTAGE_ERROR_SECOND && m_sStatistic.u64FPS == 0 && !m_bHadID && m_bDisconnect)
	{
		setReadCharsError(ERR_LIDAR_VOLTAGE);
	}
}

void HCLidar::checkPDCurrent()
{
	if(!getErrorCode(ERR_LIDAR_FPS_INVALID, PD_ERROR_TIME_MS) && (!getErrorCode(ERR_LIDAR_SPEED_LOW, PD_ERROR_TIME_MS) || !getErrorCode(ERR_LIDAR_SPEED_HIGH, PD_ERROR_TIME_MS)))
	{
		setReadCharsError(ERR_LIDAR_PD_CURRENT);
	}
		
}

void HCLidar::checkSharkBlocked()
{
    if (m_u64StartTimeSharkBlock != 0)
    {
        UINT64 endTime = HCHead::getCurrentTimestampUs();
        UINT64 u64Temp = endTime - m_u64StartTimeSharkBlock;

        if(u64Temp >= (m_sSDKPara.iBlockContinueMS+2000)*1000) //3.5+2S
        {
            m_u64StartTimeSharkBlock = 0;
            m_iSharkBlockCount=0;
        }
        else if(u64Temp >= m_sSDKPara.iBlockContinueMS*1000 && u64Temp < (m_sSDKPara.iBlockContinueMS+2000)*1000)//3.5 - 4.5s
        {
            if(m_iSharkBlockCount>0)
            {
				//LOG_ERROR("Shark moto block\n");
                setReadCharsError(ERR_SHARK_MOTOR_BLOCKED);
                m_u64StartTimeSharkBlock = 0;
                m_iSharkBlockCount=0;
            }
            else
            {
                m_u64StartTimeSharkBlock = 0;
                m_iSharkBlockCount=0;
            }

        }
        else if(u64Temp < 100*1000)//100ms
        {
            m_u64StartTimeSharkBlock = endTime;
        }
        else if(u64Temp >= MCU_BLOCK_TIME_MS*1000)
        {
            m_iSharkBlockCount++;
        }
        else
        {
        }
    }
    else
    {

        m_u64StartTimeSharkBlock = HCHead::getCurrentTimestampUs();
        m_iSharkBlockCount=0;

    }
}


void HCLidar::checkSharkInvalidPoints(tsPointCloud& sData)
{
    if (!sData.bValid)
    {
        if (m_u64StartTimeInvalidPoints != 0)
        {
            int64_t endTime = HCHead::getCurrentTimestampUs();
            if ((endTime - m_u64StartTimeInvalidPoints) >= m_sSDKPara.iCoverContinueMS*1000)
            {
                setReadCharsError( ERR_SHARK_INVALID_POINTS);
                m_u64StartTimeInvalidPoints = 0;
            }
        }
        else
        {
            m_u64StartTimeInvalidPoints = HCHead::getCurrentTimestampUs();
        }
    }
    else
    {
        m_u64StartTimeInvalidPoints = 0;
    }
}

/*
void HCLidar::getScanData(tsNodeInfo * nodebuffer, size_t buffLen, size_t &count, bool bReverse)
{
    if (nodebuffer == nullptr || buffLen == 0)
    {
        count = 0;
        return;
    }

    grabScanData(nodebuffer, buffLen, count);

    for (int i = 0; i < count; ++i)
    {
        double angle_cur = nodebuffer[i].angle_q6_checkbit;
        angle_cur = angle_cur / 64.0;
        if (angle_cur > 360)
        {
            angle_cur -= 360;
        }
        else if (angle_cur < 0)
        {
            angle_cur += 360;
        }

        if (bReverse)
        {
            angle_cur = 360 - angle_cur;
        }

        nodebuffer[i].angle_q6_checkbit = angle_cur * 64;
    }
}
*/

bool HCLidar::getScanData(std::list<tsNodeInfo>& dataList, bool bReverse)
{

    grabScanData(dataList);

	if (dataList.size() == 0)
		return false;

    for (auto it = dataList.begin(); it != dataList.end(); ++it)
    {
        double angle_cur = it->angle_q6_checkbit;
        angle_cur = angle_cur / 64.0;
        if (angle_cur > 360)
        {
            angle_cur -= 360;
        }
        else if (angle_cur < 0)
        {
            angle_cur += 360;
        }

        if (bReverse)
        {
            angle_cur = 360 - angle_cur;
        }

        it->angle_q6_checkbit = angle_cur * 64;
    }
	return true;
}



void HCLidar::grabScanDataWithLoop(std::list<tsNodeInfo>& nodeList, tsNodeInfo* nodebuffer, size_t buffLen)
{
    int len = nodeList.size();

	if (len == 0)
		return;

    double perAngle = 360.0 / nodeList.size();
    for (int i = 0; i < len; ++i)
    {
        nodebuffer[i].angle_q6_checkbit = perAngle * (i + 1.0) * 64;
        nodebuffer[i].distance_q2 = 0;
        nodebuffer[i].isValid = 0;
        nodebuffer[i].syn_quality = 0;
        //printf("%.3f,%d,%d\n", nodebuffer[i].angle_q6_checkbit, nodebuffer[i].distance_q2, nodebuffer[i].isValid);
    }

    for (auto it = nodeList.begin(); it != nodeList.end(); ++it)
    {
        if (it->isValid == 1)
        {
            int index = (it->angle_q6_checkbit / (perAngle * 64.0)) + 0.5 - 1;
            //printf("index:%d\n", index);
            pushValidData2Buffer(*it, index, nodebuffer, len);
        }
    }

    if (!checkBufferIsSorted(nodebuffer, len))
    {
        //printf("data is not sorted.\n");
        //std::stable_sort(nodebuffer, nodebuffer + len);
		std::stable_sort(nodebuffer, nodebuffer + len, nodeComparator);
    }
}

void HCLidar::pushValidData2Buffer(tsNodeInfo& nodeInfo, int index, tsNodeInfo* nodebuffer, int len)
{
    bool isExit = false;
    while (!isExit)
    {
        index = index % len;
        if (nodebuffer[index].isValid == 0)
        {
            nodebuffer[index] = nodeInfo;
            isExit = true;
        }

        ++index;
    };
}

bool HCLidar::checkBufferIsSorted(tsNodeInfo* nodebuffer, int len)
{
    if (len < 2)
    {
        return true;
    }

    for (int i = 1; i < len; ++i)
    {
        if (nodebuffer[i - 1].angle_q6_checkbit > nodebuffer[i].angle_q6_checkbit)
        {
            return false;
        }
    }

    return true;
}

void HCLidar::grabScanDataWithNoLoop(std::list<tsNodeInfo>& nodeList, tsNodeInfo* nodebuffer, size_t buffLen)
{
    size_t index = 0;
    size_t indexCount = 0;
    size_t startIndex = 0;
    size_t nodeLen = nodeList.size();

	if (nodeLen == 0)
		return;

    if (nodeLen > buffLen)
    {
        startIndex = nodeLen - buffLen;
    }

    for (auto it = nodeList.begin(); it != nodeList.end(); ++it, ++indexCount)
    {
        if (indexCount >= startIndex)
        {
            nodebuffer[index] = *it;
            ++index;
        }
    }
}

void HCLidar::grabScanData(tsNodeInfo * nodebuffer, size_t buffLen, size_t &count)
{
    std::list<tsNodeInfo> nodeList;
    {
        std::lock_guard<std::mutex> guard(m_mtxData);
        nodeList.swap(m_sNodeList);
    }

    count = nodeList.size();
    if (count == 0)
    {
        return;
    }

    if (m_bGetLoopData)
    {
        if (count > buffLen)
        {
			//LOG_ERROR("Rx buffer is too small\n" );
            setReadCharsError(ERR_RECEIVE_BUFFER_SMALL);
            count = 0;
            return;
        }

        grabScanDataWithLoop(nodeList, nodebuffer, buffLen);
    }
    else
    {
        grabScanDataWithNoLoop(nodeList, nodebuffer, buffLen);
    }

}

void HCLidar::grabScanData(std::list<tsNodeInfo>& dataList)
{
    std::list<tsNodeInfo> nodeList;
    {
        std::lock_guard<std::mutex> guard(m_mtxData);
        nodeList.swap(m_sNodeList);
    }

    if (nodeList.empty())
    {
        return;
    }

    size_t count = nodeList.size();
    tsNodeInfo* nodebuffer = new tsNodeInfo[count];
    if (m_bGetLoopData)
    {
        grabScanDataWithLoop(nodeList, nodebuffer, count);
    }
    else
    {
        grabScanDataWithNoLoop(nodeList, nodebuffer, count);
    }

    for (int i = 0; i < count; ++i)
    {
        dataList.push_back(nodebuffer[i]);
    }

    delete[] nodebuffer;

}

void HCLidar::convertDistQ2(LstPointCloud& lstG)
{
    for(auto sInfo:lstG)//for (int i = 0; i < rdPrPack_.data_num_per_pack_; ++i)
    {
        tsNodeInfo node_cur;
        m_dAngleCur = sInfo.dAngleRaw;
        // Compensate angle & dist
        unsigned int dist = sInfo.u16Dist;

        if (sInfo.bValid)
        {
            // valid data
            node_cur.distance_q2 = (uint16_t)(sInfo.u16Dist * 4);

            node_cur.isValid = 1;
            ++m_iValidNumber;
        }
        else
        {
            // invalid data
            node_cur.distance_q2 = 0;
            node_cur.isValid = 0;
        }

        node_cur.angle_q6_checkbit = (uint16_t)(sInfo.dAngle * 64);
        node_cur.syn_quality = sInfo.u16Gray;

        if (m_dAnglePre > m_dAngleCur)
        {
            m_bTurn = true;
            m_bFirsLoop = true;

            checkInvalidLidarNumber(m_iValidNumber);
            m_iValidNumber = 0;
        }
        //printf("%.4f,%.4f\n", preAngle, angle_cur);
        if (!m_bFirsLoop)
        {
            m_dAnglePre = m_dAngleCur;
            continue;
        }

        if (!m_bGetLoopData)
        {
            pushDataWithNoLoopMode(node_cur);
        }
        else
        {
            pushDataWithLoopMode(m_bTurn, m_sLoopNodeList, node_cur);
        }

        m_dAnglePre = m_dAngleCur;
        //printf("%.3f\n", node_cur.angle_q6_checkbit /  64.0f);
    }
}


void HCLidar::checkInvalidLidarNumber(int validNumber)
{
    if (validNumber < VALID_NUMBER_COUNT)
    {
        ++m_iInvalidNumberContinue;
        if (m_iInvalidNumberContinue >= NUMBER_CONTINUE_CIRCLE)
        {
            setReadCharsError(ERR_LIDAR_NUMBER_INVALID);
            m_iInvalidNumberContinue = 0;

			checkPDCurrent();
        }
    }
    else
    {
        m_iInvalidNumberContinue = 0;
    }
}
void HCLidar::pushDataWithNoLoopMode(tsNodeInfo& node_cur)
{
    //std::lock_guard<std::mutex> guard(mutex_);
    if (m_sNodeList.size() >= m_sSDKPara.iPollBuffSize)
    {
        m_sNodeList.pop_front();
        setReadCharsError(ERR_BUFF_FULL);
    }

    m_sNodeList.push_back(node_cur);
}

//isGetLoopData is false
void HCLidar::pushDataWithLoopMode(bool& bTurn, std::list<tsNodeInfo>& loopNodeList, tsNodeInfo& node_cur)
{
    if (bTurn)
    {
        if (!m_bGreaterThan || (m_bGreaterThan && loopNodeList.size() > LESS_THAN_NUMBER))
        {
            //std::lock_guard<std::mutex> guard(mutex_);
            m_sNodeList.clear();
            m_sNodeList.swap(loopNodeList);
        }
        else
        {
            loopNodeList.clear();
        }

        bTurn = false;
        m_bGreaterThan = false;

    }
    else if (loopNodeList.size() >= m_sAttr.dCirclePoints)
    {
        {
            //std::lock_guard<std::mutex> guard(mutex_);
            m_sNodeList.clear();
            m_sNodeList.swap(loopNodeList);
        }

        m_bGreaterThan = true;
    }
    loopNodeList.push_back(node_cur);
}

void HCLidar::callbackDistQ2()
{
    LstNodeDistQ2 dataList;
    getScanData(dataList,true);

    if(dataList.size()>0)
        m_funDistQ2(dataList);
}


void HCLidar::pollModePointCloud()
{
	//LOG_INFO("pollModePointCloud\n");
	if (m_bDistQ2)
	{
		if (m_resultRange.size() > 0)
		{
			convertDistQ2(m_resultRange);
			LstPointCloud tmp;
			tmp.swap(m_resultRange);
		}
	}
	else
	{
		if (m_bCircle)
		{
			if (getOneCircleData())
			{
                std::vector<W_DataScan> showData;
                static uint16_t S_F_last = filter_select;
                if (S_F_last != filter_select)
                    showData.clear();
                if (filter_select != 0)
                {
                    std::vector<std::vector<PointData>> out;
                    std::vector<PointData> new_club;
                    for (auto Point : m_lstCircle)
                    {
                        PointData tda;
                        tda.distance = Point.u16Dist;
                        tda.angle = (float)Point.dAngle;
                        tda.confidence = (uint8_t)Point.u16Gray;
                        tda.timeStamp = Point.u16Speed;
                        new_club.push_back(tda);
                    }
                    out.push_back(new_club);
                    new_club.clear();

                    if ((filter_select & (filter_smooth & 0xffff)) != 0)
                    {
                        para_inf_.filter_type = FullScanFilter::FS_Smooth;
                        full_scan_filter_.filter(out.back(), para_inf_, new_club);
                        out.push_back(new_club);
                        new_club.clear();
                        // qDebug() << "FS_Smooth";
                    }

                    if ((filter_select & (filter_bilateral & 0xffff)) != 0)
                    {
                        para_inf_.filter_type = FullScanFilter::FS_Bilateral;
                        full_scan_filter_.filter(out.back(), para_inf_, new_club);
                        out.push_back(new_club);
                        new_club.clear();
                        // qDebug() << "FS_Bilateral";
                    }

                    if ((filter_select & (filter_tail & 0xffff)) != 0)
                    {
                        para_inf_.filter_type = FullScanFilter::FS_Tail;
                        full_scan_filter_.filter(out.back(), para_inf_, new_club);
                        out.push_back(new_club);
                        new_club.clear();
                        // qDebug() << "FS_Tail";
                    }

                    if ((filter_select & (filter_intensity & 0xffff)) != 0)
                    {
                        para_inf_.filter_type = FullScanFilter::FS_Intensity;
                        full_scan_filter_.filter(out.back(), para_inf_, new_club);
                        out.push_back(new_club);
                        new_club.clear();
                        // qDebug() << "FS_Intensity";
                    }

                    Tofbf tofbf(m_lstCircle.front().u16Speed*6);
                    if ((filter_select & (filter_near & 0xffff)) != 0)
                    {
                        new_club = tofbf.NearFilter(out.back());
                        out.push_back(new_club);
                        new_club.clear();
                    }
                    if ((filter_select & (filter_noise & 0xffff)) != 0)
                    {
                        new_club = tofbf.NoiseFilter(out.back());
                        out.push_back(new_club);
                        new_club.clear();
                    }
                    if ((filter_select & (filter_tine & 0xffff)) != 0)
                    {
                        new_club = tofbf.TineFilter(out.back());
                        out.push_back(new_club);
                        new_club.clear();
                    }
                    if ((filter_select & (filter_wander & 0xffff)) != 0)
                    {
                        new_club = tofbf.WanderFilter(out.back());
                        out.push_back(new_club);
                        new_club.clear();
                    }
                    if ((filter_select & (filter_shadows & 0xffff)) != 0)
                    {
                        new_club = tofbf.ShadowsFilter(out.back());
                        out.push_back(new_club);
                        new_club.clear();
                    }
                    if ((filter_select & (filter_median & 0xffff)) != 0)
                    {
                        new_club = tofbf.MedianFilter(out.back());
                        out.push_back(new_club);
                        new_club.clear();
                    }
                    for (auto Point : out.back())
                    {
                        W_DataScan m_sData;
                        m_sData.ranges_ = (float)Point.distance;
                        m_sData.angles_ = (float)Point.angle;
                        m_sData.intensity_ = (uint8_t)Point.confidence;
                        m_sData.speed = Point.timeStamp;
                        showData.push_back(m_sData);
                    }
                }
                else
                {
                    if (S_F_last != filter_select)
                        showData.clear();

                    for (auto Point : m_lstCircle)
                    {
                        W_DataScan m_sData;
                        m_sData.ranges_ = (float)Point.u16Dist;
                        m_sData.angles_ = (float)Point.dAngle;
                        m_sData.intensity_ = (uint8_t)Point.u16Gray;
                        m_sData.speed = Point.u16Speed;
                        showData.push_back(m_sData);
                    }
                }
                S_F_last = filter_select;
                if (!showData.empty())
                {
                    if (DataFun != nullptr)
                        DataFun(showData);
                }
                // m_Circles.push_back(m_lstCircle);
                LstPointCloud tmp;
                tmp.swap(m_lstCircle);
				
			}
			if (m_Circles.size() > m_sSDKPara.iCirclesBuffSize)
			{
				m_Circles.erase(m_Circles.begin());
				setReadCharsError(ERR_BUFF_FULL);
			}
		}
		else
		{
			if (m_resultRange.size() > m_sSDKPara.iPollBuffSize)
			{
				HCHead::eraseRangeData(m_resultRange, m_resultRange.size() - m_sSDKPara.iPollBuffSize);
				setReadCharsError(ERR_BUFF_FULL);
			}
		}
	}
}
void HCLidar::callBackFunPointCloud()
{
	if (m_bDistQ2)
	{
		if (m_resultRange.size() > 0)
		{
			convertDistQ2(m_resultRange);
			LstPointCloud tmp;
			tmp.swap(m_resultRange);
		}
		if (m_sNodeList.size() >= m_sSDKPara.iCallbackBuffSize)
		{

			if (m_funDistQ2)
			{
				callbackDistQ2();
			}
			else
			{
				setReadCharsError(ERR_CALLBACK_FUN);
			}
		}
	}
	else
	{
		if (m_bCircle)
		{
			if (getOneCircleData())
			{
				if (m_funPointCloud)
				{
					m_funPointCloud(m_lstCircle);
				}
				else
				{
					setReadCharsError(ERR_CALLBACK_FUN);
				}
				LstPointCloud tmp;
				tmp.swap(m_lstCircle);
			}
		}
		else
		{
			if (m_resultRange.size() >= m_sSDKPara.iCallbackBuffSize)
			{
				if (m_funPointCloud)
				{
					m_funPointCloud(m_resultRange);
				}
				else
				{
					setReadCharsError(ERR_CALLBACK_FUN);
				}
				LstPointCloud tmp;
				tmp.swap(m_resultRange);
			}
		}
	}

	
}

bool HCLidar::getOneCircleData()
{
	bool bHadOne = false;
	int iCount = 0;
	for (auto sInfo : m_resultRange)
	{
		m_dAngleCur = sInfo.dAngleRaw;
		if(sInfo.bValid)
			++m_iValidNumber;

		if (m_lstCircle.size()>0)
		{
			auto sTemp = *m_lstCircle.begin();
			double dTemp = sTemp.dAngleRaw - m_dAngleCur;
			if ((dTemp <= m_sAttr.dAngleStep && dTemp>0) || (m_lstCircle.size()>= m_sAttr.dCirclePoints))
			{
				bHadOne = true;
				iCount++;
				m_lstCircle.push_back(sInfo);
				m_resultRange.erase(m_resultRange.begin(), m_resultRange.begin() + iCount);
				checkInvalidLidarNumber(m_iValidNumber);
				m_iValidNumber = 0;
				break;
			}	
			
		}

		iCount++;

		m_lstCircle.push_back(sInfo);
		
		m_dAnglePre = m_dAngleCur;	
	}

	if (bHadOne)
	{
		//std::stable_sort(m_lstCircle.begin(), m_lstCircle.end(), newComparator);		
	}
	else
	{
		LstPointCloud tmp;
		tmp.swap(m_resultRange);
	}
	
	return bHadOne;
}


bool HCLidar::getErrorCode(int iError, int iMs)
{
	std::lock_guard<std::mutex> lock(m_mtxError);
	std::map<int, UINT64>::iterator it = m_mapErrorCode.find(iError);
	if (it != m_mapErrorCode.end())
	{
		UINT64 u64TS = it->second;
		UINT64 u64EndUs = HCHead::getCurrentTimestampUs();
		UINT64 u64Int = u64EndUs - u64TS;

		if (u64Int <= iMs*1000)
		{
			return true;
		}
	}
	return false;
}

void HCLidar::setLidarPowerOn(bool bPowerOn)
{
	m_bDisconnect = !bPowerOn;
}

///////////////////////////////
bool HCLidar::rockCheckLDSInfo(UINT8* buffer, unLidarInfo& lds_info)
{

	int size = 0;
	int decode_size = 0;

	UINT8 decode_buff[RCV_SIZE];

	rockDecodeLDSInfoPacket(buffer, decode_buff, decode_size);

	if (decode_size < sizeof(unLidarInfo))
	{
		//qDebug() << "Fail to Check LDS info, decoded buff size error.";
		return false;
	}

	memcpy(&lds_info, &decode_buff, sizeof(unLidarInfo));

	if (!rockCheckCRC(lds_info))
	{
		//qDebug() << "Wrong CRC for LDS info.";
		return false;
	}

	//qDebug() << "Get LDS Info success";

	return true;
}

void HCLidar::rockDecodeLDSInfoPacket(UINT8* src, UINT8* dest, int& decode_size)
{
	decode_size = 0;
	for (int i = 0; i < RCV_SIZE; i++)
	{
		if (src[i] == (LDS_INFO_START - 1))
		{
			if (i < RCV_SIZE - 1)
			{
				dest[decode_size++] = (src[i + 1] == 0x01 ? LDS_INFO_START : (LDS_INFO_START - 1));
				i++;
			}
		}
		else
		{
			dest[decode_size++] = src[i];
		}
	}
}

bool HCLidar::rockCheckCRC(unLidarInfo lds_info)
{
	UINT16 u16CheckSum = 0;
	for (int i = 0; i < (sizeof(unLidarInfo) / sizeof(UINT16) - 1); i++)
	{
		u16CheckSum += lds_info.u16DataHex[i];
	}
	if (u16CheckSum != lds_info.sAttr.u16CheckSum)
	{
		//qDebug() << "LDS info checksum: 0x" << lds_info.lds_attributes.check_sum << " VS  Count_result: 0x" << checksum;
		return false;
	}

	return true;
}



void HCLidar::checkChangeSpeed()
{
	if (!m_bCheckSpeed)
	{
		return;
	}

	if (m_sStatistic.dRMS >= m_sAttr.iSpeedMin && m_sStatistic.dRMS <= m_sAttr.iSpeedMax)
	{
		m_iCheckSpeedCount = 0;
	}
	else
	{
		m_iCheckSpeedCount++;
	}

	UINT64 endTime = HCHead::getCurrentTimestampUs();
	UINT64 u64Temp = endTime - m_u64StartTimeCheckSpeed;

	if (u64Temp >= (m_sSDKPara.iChangeSpeedMS ) * 1000) //2.5S
	{
		if (m_iCheckSpeedCount>10)
		{
			setReadCharsError(ERR_LIDAR_CHANGE_SPEED);
		}

		
		m_bCheckSpeed = false;
		m_u64StartTimeCheckSpeed = 0;
	}

}

void HCLidar::resetParam()
{
	LOG_INFO("resetParam\n");
	m_iInvalidFPSSecond = 0;
	m_u64StartTimeLowSpeed = 0;
	m_u64StartTimeHighSpeed = 0;
	m_u64StartTimeSharkBlock = 0;
	m_iSharkBlockCount = 0;
	m_u64StartTimeInvalidPoints = 0;
	m_iValidNumber = 0;

	m_iLastErrorCode = 0;
}


void HCLidar::setLidarLowSpeed(bool bLow)
{
	LOG_INFO("Change speed\n");
	if (bLow)
	{
		if (m_strLidarModel == X2YE)
		{
			if (m_sAttr.dAngleStep != ANGLE_RESOLV_NARWAL_LOW)
			{
				m_bCheckSpeed = true;
				m_u64StartTimeCheckSpeed = HCHead::getCurrentTimestampUs();
			}

			m_sAttr.iSpeedMax = SPEED_250_MAX;
			m_sAttr.iSpeedMin = SPEED_250_MIN;
			m_sAttr.dAngleStep = ANGLE_RESOLV_NARWAL_LOW;
			m_sAttr.dCirclePoints = CICRLE_MAX_NARWAL_LOW;

			UCHAR u8Temp[5] = { 0xAA,0x55,0xF1,0x0E };
            // m_serial.writeData2(u8Temp, 4);
		}
		else if (m_strLidarModel == X2BF)
		{
			if (m_sAttr.dAngleStep != ANGLE_RESOLV_3000_3HZ)
			{
				m_bCheckSpeed = true;
				m_u64StartTimeCheckSpeed = HCHead::getCurrentTimestampUs();
			}

			m_sAttr.iSpeedMax = SPEED_180_MAX;
			m_sAttr.iSpeedMin = SPEED_180_MIN;
			m_sAttr.dAngleStep = ANGLE_RESOLV_3000_3HZ;
			m_sAttr.dCirclePoints = CICRLE_MAX_3000_3HZ;

		}
		else if (m_strLidarModel == X2MF)
		{
			if (m_sAttr.dAngleStep != ANGLE_RESOLV_2000_3HZ)
			{
				m_bCheckSpeed = true;
				m_u64StartTimeCheckSpeed = HCHead::getCurrentTimestampUs();
			}

			m_sAttr.iSpeedMax = SPEED_180_MAX;
			m_sAttr.iSpeedMin = SPEED_180_MIN;
			m_sAttr.dAngleStep = ANGLE_RESOLV_2000_3HZ;
			m_sAttr.dCirclePoints = CICRLE_MAX_2000_3HZ;

		}
		
	}
	else
	{
		if (m_strLidarModel == X2YE)
		{
			if (m_sAttr.dAngleStep != ANGLE_RESOLV_NARWAL_NOR)
			{
				m_bCheckSpeed = true;
				m_u64StartTimeCheckSpeed = HCHead::getCurrentTimestampUs();
			}


			m_sAttr.iSpeedMax = SPEED_315_MAX;
			m_sAttr.iSpeedMin = SPEED_315_MIN;
			m_sAttr.dAngleStep = ANGLE_RESOLV_NARWAL_NOR;
			m_sAttr.dCirclePoints = CICRLE_MAX_NARWAL_NOR;

			UCHAR u8Temp[5] = { 0xAA,0x55,0xF2,0x0D };
            // m_serial.writeData2(u8Temp, 4);
		}
		else if (m_strLidarModel == X2BF)
		{
			if (m_sAttr.dAngleStep != ANGLE_RESOLV_3000)
			{
				m_bCheckSpeed = true;
				m_u64StartTimeCheckSpeed = HCHead::getCurrentTimestampUs();
			}


			m_sAttr.iSpeedMax = SPEED_360_MAX;
			m_sAttr.iSpeedMin = SPEED_360_MIN;
			m_sAttr.dAngleStep = ANGLE_RESOLV_3000;
			m_sAttr.dCirclePoints = CICRLE_MAX_3000;
		}
		else if (m_strLidarModel == X2MF)
		{
			if (m_sAttr.dAngleStep != ANGLE_RESOLV_2000_6HZ)
			{
				m_bCheckSpeed = true;
				m_u64StartTimeCheckSpeed = HCHead::getCurrentTimestampUs();
			}

			m_sAttr.iSpeedMax = SPEED_360_MAX;
			m_sAttr.iSpeedMin = SPEED_360_MIN;
			m_sAttr.dAngleStep = ANGLE_RESOLV_2000_6HZ;
			m_sAttr.dCirclePoints = CICRLE_MAX_2000_6HZ;

		}
	}
}
