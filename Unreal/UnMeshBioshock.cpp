#include "Core.h"
#include "UnrealClasses.h"
#include "UnHavok.h"


#if BIOSHOCK


/*-----------------------------------------------------------------------------
	Bioshock Havok structures
	Mostly for Havok-4.1.0-r1
-----------------------------------------------------------------------------*/

struct hkBone
{
	const char				*m_name;
	hkBool					m_lockTranslation;
};

struct hkSkeleton
{
	const char				*m_name;
	hkInt16					*m_parentIndices;
	hkInt32					m_numParentIndices;
	hkBone					**m_bones;
	hkInt32					m_numBones;
	hkQsTransform			*m_referencePose;
	hkInt32					m_numReferencePose;
};

// classes without implementation
class hkRagdollInstance;
class hkSkeletonMapper;
class ap4AnimationPackageAnimation;		//?? implement later

struct ap4AnimationPackageRoot
{
	hkSkeleton				*m_highBoneSkeleton;
	hkRagdollInstance		*m_masterRagdollInstance;
	hkSkeletonMapper		*m_highToLowBoneSkeletonMapper;
	hkSkeletonMapper		*m_lowToHighBoneSkeletonMapper;
	ap4AnimationPackageAnimation *m_animations;
	hkInt32					m_numAnimations;
};


/*-----------------------------------------------------------------------------
	Bioshock's USkeletalMesh
-----------------------------------------------------------------------------*/

//!! NOTE: Bioshock uses EXACTLY the same rigid/smooth vertex data as older UE3.
//!! Should separate vertex declarations into h-file and use here
#if 1
struct FRigidVertexBio	//?? same layout as FRigidVertex3
{
	FVector				Pos;
	int					Normal[3];
	float				U, V;
	byte				BoneIndex;

	friend FArchive& operator<<(FArchive &Ar, FRigidVertexBio &V)
	{
		Ar << V.Pos << V.Normal[0] << V.Normal[1] << V.Normal[2];
		Ar << V.U << V.V;
		Ar << V.BoneIndex;
		return Ar;
	}
};
#else
#define FRigidVertex3 FRigidVertexBio
struct FRigidVertex3
{
	FVector				Pos;
	int					Normal[3];		// FVectorComp (FVector as 4 bytes)
	float				U, V;
	byte				BoneIndex;

	friend FArchive& operator<<(FArchive &Ar, FRigidVertex3 &V)
	{
		// note: version prior 477 have different normal/tangent format (same layout, but different
		// data meaning)
		Ar << V.Pos << V.Normal[0] << V.Normal[1] << V.Normal[2];
		Ar << V.U << V.V;
		Ar << V.BoneIndex;
		return Ar;
	}
};
#endif

#if 1
struct FSmoothVertexBio	//?? same layout as FSmoothVertex3 (old version)
{
	FVector				Pos;
	int					Normal[3];
	float				U, V;
	byte				BoneIndex[4];
	byte				BoneWeight[4];

	friend FArchive& operator<<(FArchive &Ar, FSmoothVertexBio &V)
	{
		int i;
		Ar << V.Pos << V.Normal[0] << V.Normal[1] << V.Normal[2] << V.U << V.V;
		for (i = 0; i < 4; i++)
			Ar << V.BoneIndex[i] << V.BoneWeight[i];
		return Ar;
	}
};
#else
#define FSmoothVertex3 FSmoothVertexBio
struct FSmoothVertex3
{
	FVector				Pos;
	int					Normal[3];		// FVectorComp (FVector as 4 bytes)
	float				U, V;
	byte				BoneIndex[4];
	byte				BoneWeight[4];

	friend FArchive& operator<<(FArchive &Ar, FSmoothVertex3 &V)
	{
		int i;
		// note: version prior 477 have different normal/tangent format (same layout, but different
		// data meaning)
		Ar << V.Pos << V.Normal[0] << V.Normal[1] << V.Normal[2] << V.U << V.V;
		for (i = 0; i < 4; i++)
			Ar << V.BoneIndex[i] << V.BoneWeight[i];
		return Ar;
	}
};
#endif

struct FBioshockUnk1
{
	TArray<short>		f0;
	TArray<short>		fC;
	TArray<short>		f18;
	TArray<short>		f24;
	TArray<short>		f30;
	TArray<short>		f3C;
	TArray<short>		f48;
	TArray<short>		f54;

	friend FArchive& operator<<(FArchive &Ar, FBioshockUnk1 &S)
	{
		return Ar << S.f0 << S.fC << S.f18 << S.f24 << S.f30 << S.f3C << S.f48 << S.f54;
	}
};

struct FBioshockUnk2	//?? not used
{
	int					unk[16];

	friend FArchive& operator<<(FArchive &Ar, FBioshockUnk2 &S)
	{
		for (int i = 0; i < 16; i++) Ar << S.unk[i];
		return Ar;
	}
};

struct FBioshockUnk3
{
	FName				Name;
	FVector				Pos;
	int					f14;
	byte				f18;
	UObject				*f1C;

	friend FArchive& operator<<(FArchive &Ar, FBioshockUnk3 &S)
	{
		guard(FBioshockUnk3<<);
		Ar << S.Name << S.Pos << S.f14;
		if (Ar.ArLicenseeVer >= 33)
		{
			TRIBES_HDR(Ar, 0);
			Ar << S.f18;
			if (t3_hdrSV >= 1)
				Ar << S.f1C;
		}
//		printf("... UNK3 name: %s pos: %g %g %g f14: %d f18: %d\n", *S.Name, FVECTOR_ARG(S.Pos), S.f14, S.f18);
		return Ar;
		unguard;
	}
};

struct FBioshockUnk4
{
	FName				Name;			// bone name
	FVector				Pos;
	FVector				f14;
	byte				f20;
	UMaterial			*Material;

	friend FArchive& operator<<(FArchive &Ar, FBioshockUnk4 &S)
	{
		guard(FBioshockUnk4<<);
		Ar << S.Name << S.Pos << S.f14;
		if (Ar.ArLicenseeVer >= 33)
		{
			TRIBES_HDR(Ar, 0);
			Ar << S.f20;
			if (t3_hdrSV >= 1)
				Ar << S.Material;
		}
//		printf("... UNK4 name: %s pos: %g %g %g f14: %g %g %g f20: %d Mat: %s\n", *S.Name, FVECTOR_ARG(S.Pos), FVECTOR_ARG(S.f14),
//			S.f20, S.Material ? S.Material->Name : "null");
		return Ar;
		unguard;
	}
};

struct FBioshockUnk5
{
	FName				Name;
	FVector				Pos;
	UObject				*f14;
	byte				f18;
	UObject				*f1C;

	friend FArchive& operator<<(FArchive &Ar, FBioshockUnk5 &S)
	{
		guard(FBioshockUnk5<<);
		Ar << S.Name << S.Pos << S.f14;
		if (Ar.ArLicenseeVer >= 33)
		{
			TRIBES_HDR(Ar, 0);
			Ar << S.f18;
			if (t3_hdrSV >= 1)
				Ar << S.f1C;
		}
//		printf("... UNK5 name: %s pos: %g %g %g f18: %d\n", *S.Name, FVECTOR_ARG(S.Pos), S.f18);
		return Ar;
		unguard;
	}
};

struct FMeshNormalBio
{
	FVector				Normal[3];

	friend FArchive& operator<<(FArchive &Ar, FMeshNormalBio &N)
	{
		return Ar << N.Normal[0] << N.Normal[1] << N.Normal[2];
	}
};

struct FStaticLODModelBio
{
	TArray<FSkelMeshSection> Sections;			// standard format, but 1 section = 1 material (rigid and smooth verts
												// are placed into a single section)
	TArray<short>			Bones;
	FRawIndexBuffer			IndexBuffer;
	TArray<FSmoothVertexBio> SmoothVerts;
	TArray<FRigidVertexBio>	RigidVerts;
	TArray<FBioshockUnk1>	f58;
	float					f64;
	float					f68;
	int						f6C;
	int						f70;
	int						f74;
	int						f78;
	float					f7C;
	TLazyArray<FVertInfluences> VertInfluences;
	TLazyArray<FMeshWedge>	Wedges;
	TLazyArray<FMeshFace>	Faces;
	TLazyArray<FVector>		Points;				// all surface points
	TLazyArray<FMeshNormalBio> Normals;

	friend FArchive& operator<<(FArchive &Ar, FStaticLODModelBio &Lod)
	{
		guard(FStaticLODModelBio<<);
		TRIBES_HDR(Ar, 0);
		Ar << Lod.Sections << Lod.Bones << Lod.IndexBuffer;
		Ar << Lod.SmoothVerts << Lod.RigidVerts;
		Ar << Lod.f64 << Lod.f68 << Lod.f6C << Lod.f70 << Lod.f74 << Lod.f78;
		if (t3_hdrSV >= 1)
			Ar << Lod.f7C;						// default = -1.0f
		if (t3_hdrSV >= 3)
			Ar << Lod.f58;
		Ar << Lod.VertInfluences << Lod.Wedges << Lod.Faces << Lod.Points << Lod.Normals;
#if 0
		printf("sm:%d rig:%d idx:%d bones:%d 58:%d 64:%g 68:%g 6C:%d 70:%d 74:%d 78:%d\n",
			Lod.SmoothVerts.Num(), Lod.RigidVerts.Num(), Lod.IndexBuffer.Indices.Num(), Lod.Bones.Num(), Lod.f58.Num(),
			Lod.f64, Lod.f68, Lod.f6C, Lod.f70, Lod.f74, Lod.f78);
		printf("inf:%d wedg:%d fac:%d pts:%d norm:%d\n", Lod.VertInfluences.Num(), Lod.Wedges.Num(), Lod.Faces.Num(), Lod.Points.Num(), Lod.Normals.Num());
		int j;
//		for (j = 0; j < Lod.Bones.Num(); j++)
//			printf("Bones[%d] = %d\n", j, Lod.Bones[j]);
		for (j = 0; j < Lod.Sections.Num(); j++)
		{
			const FSkelMeshSection &S = Lod.Sections[j];
			printf("... SEC[%d]:  mat=%d %d [w=%d .. %d] %d b=%d %d [f=%d + %d]\n", j,
				S.MaterialIndex, S.MinStreamIndex, S.MinWedgeIndex, S.MaxWedgeIndex,
				S.NumStreamIndices, S.BoneIndex, S.fE, S.FirstFace, S.NumFaces);
		}
#endif
		return Ar;
		unguard;
	}
};

//?? check same function in UnMesh.cpp
static bool CompareCompNormals(int Normal1, int Normal2)
{
	for (int i = 0; i < 3; i++)
	{
		char b1 = Normal1 & 0xFF;
		char b2 = Normal2 & 0xFF;
		if (abs(b1 - b2) > 10) return false;
		Normal1 >>= 8;
		Normal2 >>= 8;
	}
	return true;
}

// partially based on FStaticLODModel::RestoreMesh3()
void FStaticLODModel::RestoreMeshBio(const USkeletalMesh &Mesh, const FStaticLODModelBio &Lod)
{
	guard(FStaticLODModel::RestoreMeshBio);

	int NumVertices = Lod.SmoothVerts.Num() + Lod.RigidVerts.Num();

	// prepare arrays
	TArray<int> PointNormals;
	Points.Empty        (NumVertices);
	PointNormals.Empty  (NumVertices);
	Wedges.Empty        (NumVertices);
	VertInfluences.Empty(NumVertices * 4);

	int Vert, j;

	guard(RigidVerts);
	for (Vert = 0; Vert < Lod.RigidVerts.Num(); Vert++)
	{
		const FRigidVertexBio &V = Lod.RigidVerts[Vert];
		// find the same point in previous items
		int PointIndex = -1;	// start with 0, see below
		while (true)
		{
			PointIndex = Points.FindItem(V.Pos, PointIndex + 1);
			if (PointIndex == INDEX_NONE) break;
			if (CompareCompNormals(PointNormals[PointIndex], V.Normal[0])) break;
		}
		if (PointIndex == INDEX_NONE)
		{
			// point was not found - create it
			PointIndex = Points.Add();
			Points[PointIndex] = V.Pos;
			PointNormals.AddItem(V.Normal[0]);
			// add influence
			FVertInfluences *Inf = new(VertInfluences) FVertInfluences;
			Inf->PointIndex = PointIndex;
			Inf->Weight     = 1.0f;
			Inf->BoneIndex  = Lod.Bones[V.BoneIndex];
		}
		// create wedge
		FMeshWedge *W = new(Wedges) FMeshWedge;
		W->iVertex = PointIndex;
		W->TexUV.U = V.U;
		W->TexUV.V = V.V;
	}
	unguard;

	guard(SmoothVerts);
	for (Vert = 0; Vert < Lod.SmoothVerts.Num(); Vert++)
	{
		const FSmoothVertexBio &V = Lod.SmoothVerts[Vert];
		// find the same point in previous items
		int PointIndex = -1;	// start with 0, see below
		while (true)
		{
			PointIndex = Points.FindItem(V.Pos, PointIndex + 1);
			if (PointIndex == INDEX_NONE) break;
			if (CompareCompNormals(PointNormals[PointIndex], V.Normal[0])) break;
			//?? should compare influences too !
		}
		if (PointIndex == INDEX_NONE)
		{
			// point was not found - create it
			PointIndex = Points.Add();
			Points[PointIndex] = V.Pos;
			PointNormals.AddItem(V.Normal[0]);
			// add influences
//			printf("point[%d]: %d/%d  %d/%d  %d/%d  %d/%d\n", PointIndex, V.BoneIndex[0], V.BoneWeight[0],
//				V.BoneIndex[1], V.BoneWeight[1], V.BoneIndex[2], V.BoneWeight[2], V.BoneIndex[3], V.BoneWeight[3]);
			for (int i = 0; i < 4; i++)
			{
				int BoneIndex  = V.BoneIndex[i];
				int BoneWeight = V.BoneWeight[i];
				if (BoneWeight == 0) continue;
				FVertInfluences *Inf = new(VertInfluences) FVertInfluences;
				Inf->PointIndex = PointIndex;
				Inf->Weight     = BoneWeight / 255.0f;
				Inf->BoneIndex  = Lod.Bones[BoneIndex];
			}
		}
		// create wedge
		FMeshWedge *W = new(Wedges) FMeshWedge;
		W->iVertex = PointIndex;
		W->TexUV.U = V.U;
		W->TexUV.V = V.V;
	}
	unguard;

	// count triangles (speed optimization for TArray allocations)
	int NumTriangles = 0;
	int Sec;
	for (Sec = 0; Sec < Lod.Sections.Num(); Sec++)
		NumTriangles += Lod.Sections[Sec].NumFaces;
	Faces.Empty(NumTriangles);
	// create faces
	for (Sec = 0; Sec < Lod.Sections.Num(); Sec++)
	{
		const FSkelMeshSection &S = Lod.Sections[Sec];
		FSkelMeshSection *Dst = new (SmoothSections) FSkelMeshSection;
		int MaterialIndex = S.MaterialIndex;
		Dst->MaterialIndex = MaterialIndex;
		Dst->FirstFace     = Faces.Num();
		Dst->NumFaces      = S.NumFaces;
//		printf("3a : %d/%d+%d faces, %d indices\n", S.FirstFace, Dst->FirstFace, S.NumFaces, Lod.IndexBuffer.Indices.Num());
		int Index = S.FirstFace * 3; //?? Dst->FirstFace * 3;
#if 1
		//?? Bug in few Bioshock meshes: not enough indices (usually 12 indices = 4 faces)
		//?? In that case, Dst->FirstFace == S.FirstFace - 4 (incorrect S.FirstFace value ?)
		//?? Possible solution: use "Index = Dst->FirstFace * 3" in code above
		if (Index + Dst->NumFaces * 3 > Lod.IndexBuffer.Indices.Num())
		{
			int cut = Index + Dst->NumFaces * 3 - Lod.IndexBuffer.Indices.Num();
			appNotify("Bioshock LOD: missing %d indices", cut);
			Dst->NumFaces -= cut / 3;
		}
#endif
		for (int i = 0; i < Dst->NumFaces; i++)
		{
			FMeshFace *Face = new (Faces) FMeshFace;
			Face->MaterialIndex = MaterialIndex;
			Face->iWedge[0] = Lod.IndexBuffer.Indices[Index++];
			Face->iWedge[1] = Lod.IndexBuffer.Indices[Index++];
			Face->iWedge[2] = Lod.IndexBuffer.Indices[Index++];
		}
	}

	unguard;
}


void USkeletalMesh::SerializeBioshockMesh(FArchive &Ar)
{
	guard(USkeletalMesh::SerializeBioshock);

	UPrimitive::Serialize(Ar);
	TRIBES_HDR(Ar, 0);

	float					unk90, unk94;
	int						unk98, unk9C;	//?? 9C type is unknown (float/int)
	int						f68;
	TArray<FStaticLODModelBio> bioLODModels;
	TLazyArray<FMeshNormalBio> bioNormals;
	UObject					*fCC;			//??
	TArray<FBioshockUnk3>	f104;			// count = num bones or 0
	TArray<FBioshockUnk4>	f110;			// count = num bones or 0
	TArray<FBioshockUnk5>	f11C;			// count = num bones or 0
	TArray<UObject*>		f128;			// TArray<UModel*>, count = num bones or 0
	TArray<FVector>			f134;			// count = num bones or 0
	UObject					*HkMeshProxy;	// UHkMeshProxy* - Havok data
	TArray<FName>			drop1;

	Ar << Textures << MeshScale << MeshOrigin << RotOrigin;
	Ar << unk90 << unk94 << unk98 << unk9C;
	Ar << RefSkeleton << SkeletalDepth << Animation << AttachAliases << AttachBoneNames << AttachCoords;
	Ar << bioLODModels;
	Ar << fCC;
	Ar << Points << Wedges << Triangles << VertInfluences;
	Ar << CollapseWedge << f1C8;
	Ar << bioNormals;

	SkipLazyArray(Ar);	// FMeshFace	f254
	SkipLazyArray(Ar);	// FMeshWedge	f234
	SkipLazyArray(Ar);	// short		f274
	Ar << f68 << f104 << f110 << f128 << f11C;
	Ar << HkMeshProxy;
	if (t3_hdrSV <= 5) Ar << drop1;
	if (t3_hdrSV >= 4) Ar << havokObjects;
	if (t3_hdrSV >= 5) Ar << f134;

	// convert LODs
	int i;
	LODModels.Empty(bioLODModels.Num());
	LODModels.Add(bioLODModels.Num());
	for (i = 0; i < bioLODModels.Num(); i++)
		LODModels[i].RestoreMeshBio(*this, bioLODModels[i]);

	// materials
	Materials.Add(Textures.Num());
	for (i = 0; i < Materials.Num(); i++)
		Materials[i].TextureIndex = i;

	// convert LOD 0 to mesh
	RecreateMeshFromLOD();

#if 0
printf("u104:%d u110:%d u128:%d u11C:%d dr1:%d dr2:%d f134:%d\n", f104.Num(), f110.Num(), f128.Num(), f11C.Num(), drop1.Num(), havokObjects.Num(), f134.Num());
for (i=0;i<drop1.Num();i++)printf("drop[%d]=%s\n",i,*drop1[i]);
for (int j = 0; j < 10; j++)
{
for (i = 0; i < 16; i++)
{
byte b; Ar << b;
printf("  %02X",b);
}
printf("\n");
}
printf("skel: %d (depth=%d)\n", RefSkeleton.Num(), SkeletalDepth);
for (i = 0; i < RefSkeleton.Num(); i++) printf("%d: %s\n", i, *RefSkeleton[i].Name);
#endif

	unguard;
}


void USkeletalMesh::PostLoadBioshockMesh()
{
	guard(USkeletalMesh::PostLoadBioshockMesh);

	if (RefSkeleton.Num())
	{
		appNotify("Bioshock mesh %s has RefSkeleton!", Name);
		return;
	}

	int i;
	// find UAnimationPackageWrapper object
	const UAnimationPackageWrapper *AW = NULL;
	for (i = 0; i < havokObjects.Num(); i++)
	{
		UObject *Obj = havokObjects[i];
		if (!Obj) continue;
		if (Obj->IsA("AnimationPackageWrapper"))
		{
			AW = (const UAnimationPackageWrapper*)Obj;
			break;
		}
	}
	assert(AW);

	ap4AnimationPackageRoot *Object;
	const char *ClassName;
	GetHavokPackfileContents(&AW->HavokData[0], (void**)&Object, &ClassName);
	assert(!strcmp(ClassName, "ap4AnimationPackageRoot"));

	// convert skeleton
	const hkSkeleton *Skel = Object->m_highBoneSkeleton;
//	assert(RefSkeleton.Num() == 0);
	RefSkeleton.Add(Skel->m_numBones);
	for (i = 0; i < Skel->m_numBones; i++)
	{
		FMeshBone &B = RefSkeleton[i];
		B.Name.Str    = Skel->m_bones[i]->m_name;				//?? hack: FName assignment
		B.ParentIndex = max(Skel->m_parentIndices[i], 0);
		const hkQsTransform &t = Skel->m_referencePose[i];
		B.BonePos.Orientation = (FQuat&)   t.m_rotation;
		B.BonePos.Position    = (FVector&) t.m_translation;
		B.BonePos.Orientation.W *= -1;
//		if (!i) ((CQuat&)B.BonePos.Orientation).Conjugate();	//??
	}

	unguard;
}


void UAnimationPackageWrapper::Process()
{
	guard(UAnimationPackageWrapper::Process);
	FixupHavokPackfile(Name, &HavokData[0]);
#if 0
	//?? testing
	ap4AnimationPackageRoot *Object;
	const char *ClassName;
	GetHavokPackfileContents(&HavokData[0], (void**)&Object, &ClassName);
	assert(!strcmp(ClassName, "ap4AnimationPackageRoot"));
	const hkSkeleton *Skel = Object->m_highBoneSkeleton;
	printf("skel=%s\n", Skel->m_name);
	for (int i = 0; i < Skel->m_numBones; i++)
	{
		printf("[%d] %s (parent=%d)\n", i, Skel->m_bones[i]->m_name, Skel->m_parentIndices[i]);
		hkQsTransform &t = Skel->m_referencePose[i];
		printf("   {%g %g %g} - {%g %g %g %g} / {%g %g %g}\n", FVECTOR_ARG(t.m_translation), FQUAT_ARG(t.m_rotation), FVECTOR_ARG(t.m_scale));
	}
#endif
	unguard;
}


#endif // BIOSHOCK
