struct VSInput
{
	float3 position : POSITION;
};

struct VSOutput
{
	float4 position : SV_POSITION;
};

VSOutput main(VSInput input)
{
	VSOutput output;

	// CPlanMesh is generated on the XZ plane in the [-1, 1] range.
	// Send it directly to clip space without world/view/projection matrices.
	output.position = float4(input.position.x, input.position.z, 0.0f, 1.0f);
	return output;
}
