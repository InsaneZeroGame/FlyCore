#pragma once
#include "D3D12Resource.h"

namespace Renderer
{
	class D3D12Texture :  public D3D12Resource
	{
	public:

		~D3D12Texture();

		__forceinline const D3D12Descriptor* GetSRV()
		{
			return m_srv;
		}

	protected:

		D3D12Texture(const D3D12_RESOURCE_DESC& p_desc, D3D12_HEAP_FLAGS p_flag, D3D12_CLEAR_VALUE p_clearValue = {}, ID3D12Resource* p_resource = nullptr, D3D12_RESOURCE_STATES p_state = D3D12_RESOURCE_STATE_COMMON);

		D3D12_RESOURCE_DESC m_resourceDesc;

		const D3D12Descriptor* m_srv;
		
		virtual void CreateViews() {};
	};

	class D3D12Texture2D : public D3D12Texture
	{
	public:
		D3D12Texture2D(uint32_t p_width, uint32_t p_height,DXGI_FORMAT p_format = DXGI_FORMAT_R8G8B8A8_UNORM);
		~D3D12Texture2D();

	private:
		void CreateViews() override;
	};


	class D3D12RenderTarget : public D3D12Texture
	{
	public:
		D3D12RenderTarget(uint32_t p_width, uint32_t p_height, ID3D12Resource* p_resouarce,DXGI_FORMAT p_format);
		~D3D12RenderTarget();

		__forceinline const D3D12Descriptor* GetRTV()
		{
			return m_rtv;
		}

	private:
		void CreateViews() override;

		const D3D12Descriptor* m_rtv;
	};

	



	class D3D12DepthBuffer final: public D3D12Texture
	{
	public:
		D3D12DepthBuffer(uint32_t p_width, uint32_t p_height);
		
		~D3D12DepthBuffer();

		__forceinline const D3D12Descriptor* GetDSV()
		{
			return m_dsv;
		}
	private:
		void CreateViews() override;

		const D3D12Descriptor* m_dsv;

	};
	
}