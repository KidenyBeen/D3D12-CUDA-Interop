#include "../Common/Common.hlsli"

float4 main(float4 position : SV_POSITION) : SV_TARGET
{
	const float2 screenSize = float2(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
	const float2 uv = position.xy / screenSize;

	// MainScene RootParameter 4번에 연결된 t1 Texture를 전체 화면에 출력한다.
	return gtxtTextureAlbedo.Sample(glinearSamplerState, uv);
}
