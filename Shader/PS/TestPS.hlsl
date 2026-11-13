#include "../Common/Common.hlsli"

float4 main(float4 position : SV_POSITION) : SV_TARGET
{
	const float2 screenSize = float2(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
	const float shortestScreenSize = min(screenSize.x, screenSize.y);
	const float circleRadiusRatio = 0.35f;
	const float4 circleColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
	const float4 backgroundColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	// 픽셀 좌표를 UV로 바꾼 뒤 화면 중심으로 이동한다.
	const float2 uv = position.xy / screenSize;
	const float2 centeredPixelPosition = (uv - 0.5f) * screenSize;

	// 반지름은 화면의 짧은 변을 기준으로 하므로 화면 비율이 달라도 원형을 유지한다.
	const float radius = shortestScreenSize * circleRadiusRatio;
	const bool isInsideCircle = (length(centeredPixelPosition) <= radius);

	// CUDA 블러 결과를 쉽게 확인할 수 있도록 원은 빨간색, 바깥은 검은색으로 유지한다.
	return isInsideCircle ? circleColor : backgroundColor;
}
