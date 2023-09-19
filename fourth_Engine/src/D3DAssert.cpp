#include "pch.h"

#pragma once
#include "include/D3DAssert.h"

#include "include/win_def.h"
#include <dxgidebug.h>
#include "include/win_undef.h"
#include "include/Render/Renderer/D3DRenderer.h"

namespace fth
{
	DxDebugParser::DxDebugParser() = default;
	DxDebugParser::~DxDebugParser()
	{
		
	}

	void DxDebugParser::InitInternal()
	{

	
		FTH_ASSERT_HR(DXGIGetDebugInterface1(0, __uuidof(IDXGIInfoQueue), (void**)m_dxgiInfoQueue.reset()));
		//Pushing this empty filter logs every single message from D3D layer
		FTH_ASSERT_HR(m_dxgiInfoQueue->PushEmptyStorageFilter(DXGI_DEBUG_ALL));


	}

	void DxDebugParser::ShutdownInternal()
	{
		m_dxgiInfoQueue.release();
	}

	bool DxDebugParser::QueryMessagesWithSeverityInternal()
	{
		m_messages.clear();

		bool shouldBreak{};
		UINT64 numMSG = m_dxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
		if (numMSG > 0)
		{
			m_messages.reserve(numMSG);
			for (auto i = 0; i < numMSG; i++)
			{

				SIZE_T messageLength{};
				// get the size of message i in bytes

				FTH_ASSERT_HR(m_dxgiInfoQueue->GetMessageW(DXGI_DEBUG_ALL, i, nullptr, &messageLength));
				// allocate memory for message, this could throw
				DXGI_INFO_QUEUE_MESSAGE* dxgiMsg = (DXGI_INFO_QUEUE_MESSAGE*)malloc(messageLength);
				
				// get the message and push its description into the vector
				FTH_ASSERT_HR(m_dxgiInfoQueue->GetMessageW(DXGI_DEBUG_ALL, i, dxgiMsg, &messageLength));

				LogSystem::SeverityLevel severity = LogSystem::SeverityLevel::NUM;
				std::string messageToLog;

				if (dxgiMsg) //just to get rid of derreferencing  null warning(?)
					messageToLog = dxgiMsg->pDescription;
				

				switch (dxgiMsg->Severity)
				{

				//case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE:
				//{
				//	severity = LogSystem::SeverityLevel::Trace;
				//	break;
				//}
				//case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO:
				//{
				//	severity = LogSystem::SeverityLevel::Info;
				//	break;
				//}
				case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING:
				{
					severity = LogSystem::SeverityLevel::Warn;
					break;
				}
				case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR:
				{
					severity = LogSystem::SeverityLevel::Critical;
					shouldBreak = true;
					messageToLog = "D3D11 ERROR:" + messageToLog;
					break;
				}
				case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION:
				{
					severity = LogSystem::SeverityLevel::Critical;
					shouldBreak = true;
					messageToLog = "D3D11 CORRUPTION:" + messageToLog;
					break;
				}
				default:
					severity = LogSystem::SeverityLevel::NUM;
				}
				if (severity == LogSystem::SeverityLevel::NUM)
					continue;

				m_messages.emplace_back(std::make_pair(severity, messageToLog));
				free(dxgiMsg);
			}

			m_dxgiInfoQueue->ClearStoredMessages(DXGI_DEBUG_ALL);
		}
		
		return shouldBreak;
	}

}

