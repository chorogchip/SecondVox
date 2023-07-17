


cbuffer cbPerFrame : register(b0)
{
    float4x4 gViewProj;
    float4 gCamPos;
};
 
struct VertexIn
{
    uint PosYZ : POSITION;
    uint PosX_Mat_Size : MATERIAL;
};

struct VertexOut
{
	float4 Pos : POSITION;
    uint Mat_Normal : NORMAL;
};

VertexIn VS(VertexIn vin)
{
    VertexIn vout;
	
    vout.PosYZ = vin.PosYZ;
    vout.PosX_Mat_Size = vin.PosX_Mat_Size;

	return vout;
}
 
[maxvertexcount(12)]
void GS(point VertexIn gin[1],
//        uint primID : SV_PrimitiveID, 
        inout TriangleStream<VertexOut> triStream)
{	
    float4 pos_center = float4(
		(float) (gin[0].PosX_Mat_Size >> 16),
		(float) (gin[0].PosYZ >> 16),
		(float) (gin[0].PosYZ & 0xffff),
		1.0f);
	
    float4 pos_vertes[8];
	
    pos_vertes[0] = pos_center + float4(-0.5f, -0.5f, -0.5f, 0.0f);
    pos_vertes[1] = pos_center + float4( 0.5f, -0.5f, -0.5f, 0.0f);
    pos_vertes[2] = pos_center + float4(-0.5f, -0.5f,  0.5f, 0.0f);
    pos_vertes[3] = pos_center + float4( 0.5f, -0.5f,  0.5f, 0.0f);
    pos_vertes[4] = pos_center + float4(-0.5f,  0.5f, -0.5f, 0.0f);
    pos_vertes[5] = pos_center + float4( 0.5f,  0.5f, -0.5f, 0.0f);
    pos_vertes[6] = pos_center + float4(-0.5f,  0.5f,  0.5f, 0.0f);
    pos_vertes[7] = pos_center + float4( 0.5f,  0.5f,  0.5f, 0.0f);
	
    VertexOut gout[4];
    [unroll]
    for (int i = 0; i < 4; ++i)
        gout[i].Mat_Normal = gin[0].PosX_Mat_Size & 0xfff0;
    
    if (pos_center.y < gCamPos.y)
    {
        gout[0].Pos = pos_vertes[4];
        gout[1].Pos = pos_vertes[5];
        gout[2].Pos = pos_vertes[7];
        gout[3].Pos = pos_vertes[6];
        gout[0].Mat_Normal |= 0;
        gout[1].Mat_Normal |= 0;
        gout[2].Mat_Normal |= 0;
        gout[3].Mat_Normal |= 0;
    }
    else
    {
        gout[0].Pos = pos_vertes[0];
        gout[1].Pos = pos_vertes[2];
        gout[2].Pos = pos_vertes[3];
        gout[3].Pos = pos_vertes[1];
        gout[0].Mat_Normal |= 1;
        gout[1].Mat_Normal |= 1;
        gout[2].Mat_Normal |= 1;
        gout[3].Mat_Normal |= 1;
    }
    
    triStream.Append(gout[0]);
    triStream.Append(gout[1]);
    triStream.Append(gout[2]);
    triStream.Append(gout[3]);
    triStream.RestartStrip();
    gout[0].Mat_Normal &= 0xfff0;
    gout[1].Mat_Normal &= 0xfff0;
    gout[2].Mat_Normal &= 0xfff0;
    gout[3].Mat_Normal &= 0xfff0;
    
    if (pos_center.z < gCamPos.z)
    {
        gout[0].Pos = pos_vertes[0];
        gout[1].Pos = pos_vertes[1];
        gout[2].Pos = pos_vertes[5];
        gout[3].Pos = pos_vertes[4];
        gout[0].Mat_Normal |= 2;
        gout[1].Mat_Normal |= 2;
        gout[2].Mat_Normal |= 2;
        gout[3].Mat_Normal |= 2;
    }
    else
    {
        gout[0].Pos = pos_vertes[3];
        gout[1].Pos = pos_vertes[2];
        gout[2].Pos = pos_vertes[6];
        gout[3].Pos = pos_vertes[7];
        gout[0].Mat_Normal |= 3;
        gout[1].Mat_Normal |= 3;
        gout[2].Mat_Normal |= 3;
        gout[3].Mat_Normal |= 3;
    }
    
    triStream.Append(gout[0]);
    triStream.Append(gout[1]);
    triStream.Append(gout[2]);
    triStream.Append(gout[3]);
    triStream.RestartStrip();
    gout[0].Mat_Normal &= 0xfff0;
    gout[1].Mat_Normal &= 0xfff0;
    gout[2].Mat_Normal &= 0xfff0;
    gout[3].Mat_Normal &= 0xfff0;
    
    if (pos_center.x < gCamPos.x)
    {
        gout[0].Pos = pos_vertes[2];
        gout[1].Pos = pos_vertes[0];
        gout[2].Pos = pos_vertes[4];
        gout[3].Pos = pos_vertes[6];
        gout[0].Mat_Normal |= 4;
        gout[1].Mat_Normal |= 4;
        gout[2].Mat_Normal |= 4;
        gout[3].Mat_Normal |= 4;
    }
    else
    {
        gout[0].Pos = pos_vertes[1];
        gout[1].Pos = pos_vertes[3];
        gout[2].Pos = pos_vertes[7];
        gout[3].Pos = pos_vertes[5];
        gout[0].Mat_Normal |= 5;
        gout[1].Mat_Normal |= 5;
        gout[2].Mat_Normal |= 5;
        gout[3].Mat_Normal |= 5;
    }
    
    triStream.Append(gout[0]);
    triStream.Append(gout[1]);
    triStream.Append(gout[2]);
    triStream.Append(gout[3]);
    triStream.RestartStrip();
}

float4 PS(VertexOut pin) : SV_Target
{
    float3 ret_color;
	
    float3 mat_colors[3] =
    {
        float3(1.0f, 0.0f, 0.0f),
        float3(0.0f, 1.0f, 0.0f),
        float3(0.0f, 0.0f, 1.0f),
    };

    ret_color = mat_colors[pin.Mat_Normal >> 4];
	
    return float4(ret_color, 1.0f);
}


