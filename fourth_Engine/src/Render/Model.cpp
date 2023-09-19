#include "pch.h"
#pragma once
#include "include/Render/Model.h"
#include "include/Logging/AssimpLogger.h"
#include "include/Render/Renderer/D3D11Headers.h"



namespace fth
{
	Model::Model()
	{
	}

	Model::Model(std::string_view name):
		m_name(name)
	{


	}

	Model::~Model()
	{

	}


	void Model::Init(std::shared_ptr<Model> selftPtr)
	{
		m_vertices.CreateGPUBuffer();
		m_trianglesIndexed.CreateGPUBuffer();
		for (auto& mesh : m_meshes)
		{
			mesh.Initialize(selftPtr);
		}
	}

	void Model::Shutdown()
	{
		for (auto& mesh : m_meshes)
		{
			mesh.Shutdown();
		}
		m_vertices.ClearAll();
		m_trianglesIndexed.ClearAll();
	}


	bool Model::BindIndexBuffer() const
	{
		if (m_trianglesIndexed.m_cpuData.data())
		{
			m_trianglesIndexed.Bind();
			return true;
		}
		return false;
	}





}