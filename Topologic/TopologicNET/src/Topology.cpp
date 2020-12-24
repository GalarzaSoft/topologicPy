// This file is part of Topologic software library.
// Copyright(C) 2019, Cardiff University and University College London
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <msclr/marshal_cppstd.h>
#ifdef max
#undef max
#endif
#include "Topology.h"

#include <Cluster.h>
#include <CellComplex.h>
#include <Cell.h>
#include <Shell.h>
#include <Face.h>
#include <Wire.h>
#include <Edge.h>
#include <Vertex.h>
#include <Aperture.h>
#include <Context.h>
#include <TopologyFactoryManager.h>
#include <TopologyFactory.h>
#include <ApertureFactory.h>
#include <VertexFactory.h>
#include <EdgeFactory.h>
#include <WireFactory.h>
#include <FaceFactory.h>
#include <ShellFactory.h>
#include <CellFactory.h>
#include <CellComplexFactory.h>
#include <ClusterFactory.h>
#include <ApertureFactory.h>
#include <AttributeFactoryManager.h>
#include <AttributeFactory.h>
#include <TopologyUtility.h>

#include <TopologicCore/include/AttributeManager.h>
#include <TopologicCore/include/Attribute.h>
#include <TopologicCore/include/IntAttribute.h>
#include <TopologicCore/include/DoubleAttribute.h>
#include <TopologicCore/include/StringAttribute.h>

#include <map>

//using namespace System::Xml;
using namespace System::Reflection;

namespace Topologic
{
	int Topology::Dimensionality::get()
	{
		std::shared_ptr<TopologicCore::Topology> pCoreTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		return pCoreTopology->Dimensionality();
	}

#ifdef TOPOLOGIC_DYNAMO
	Topology^ Topology::ByGeometry(Autodesk::DesignScript::Geometry::Geometry^ geometry, double tolerance)
	{
		if (geometry == nullptr)
		{
			throw gcnew Exception("A null input is given.");
		}

		Topology^ topology = nullptr;

		Autodesk::DesignScript::Geometry::Point^ dynamoPoint = dynamic_cast<Autodesk::DesignScript::Geometry::Point^>(geometry);
		if (dynamoPoint != nullptr)
		{
			topology = Vertex::ByPoint(dynamoPoint);
		}

		Autodesk::DesignScript::Geometry::Curve^ dynamoCurve = dynamic_cast<Autodesk::DesignScript::Geometry::Curve^>(geometry);
		if (dynamoCurve != nullptr)
		{
			// Do this first so that a polycurve (which is a curve) is not handled by Edge.
			Autodesk::DesignScript::Geometry::PolyCurve^ dynamoPolyCurve = dynamic_cast<Autodesk::DesignScript::Geometry::PolyCurve^>(geometry);
			if (dynamoPolyCurve != nullptr)
			{
				topology = Wire::ByPolyCurve(dynamoPolyCurve);
			}
			else
			{

				// If it is a curve which actually contains more than 1 curves, create a polyCurve first, because it has a NumberOfCurves property.
				List<Autodesk::DesignScript::Geometry::Curve^>^ dynamoCurves = gcnew List<Autodesk::DesignScript::Geometry::Curve^>();
				dynamoCurves->Add(dynamoCurve);
				Autodesk::DesignScript::Geometry::PolyCurve^ dynamoNewPolyCurve = Autodesk::DesignScript::Geometry::PolyCurve::ByJoinedCurves(dynamoCurves, 0.0001);
				int numOfCurves = dynamoNewPolyCurve->NumberOfCurves;
				if (numOfCurves > 1)
				{
					Wire^ wire = Wire::ByPolyCurve(dynamoPolyCurve);
					topology = wire;
				}
				else
				{
					topology = Edge::ByCurve(dynamoCurve);
				}
				delete dynamoNewPolyCurve;

				if (numOfCurves < 1)
				{
					throw gcnew Exception("The geometry is a curve by type but no curve is detected.");
				}
			}
		}

		Autodesk::DesignScript::Geometry::Surface^ dynamoSurface = dynamic_cast<Autodesk::DesignScript::Geometry::Surface^>(geometry);
		if (dynamoSurface != nullptr)
		{
			// Do this first so that a polySurface (which is a surface) is not handled by Face.
			Autodesk::DesignScript::Geometry::PolySurface^ dynamoPolySurface = dynamic_cast<Autodesk::DesignScript::Geometry::PolySurface^>(geometry);
			if (dynamoPolySurface != nullptr)
			{
				topology = Shell::ByPolySurface(dynamoPolySurface);
			}
			else
			{
				// If it is a surface which actually contains more than 1 surfaces, create a polySurface first, because it has a SurfaceCount method.
				List<Autodesk::DesignScript::Geometry::Surface^>^ surfaces = gcnew List<Autodesk::DesignScript::Geometry::Surface^>();
				surfaces->Add(dynamoSurface);
				try {
					Autodesk::DesignScript::Geometry::PolySurface^ dynamoNewPolySurface = Autodesk::DesignScript::Geometry::PolySurface::ByJoinedSurfaces(surfaces);
					int numOfSurfaces = dynamoNewPolySurface->SurfaceCount();
					if (numOfSurfaces > 1)
					{
						// This can be a shell or a cluster, so call Topology::ByPolySurface.
						topology = Topology::ByPolySurface(dynamoNewPolySurface);
					}else if(numOfSurfaces == 1){
						topology = Face::BySurface(dynamoSurface);
					}

					delete dynamoNewPolySurface;
					if (numOfSurfaces < 1)
					{
						throw gcnew Exception("The geometry is a surface by type but no surface is detected.");
					}
				}
				catch (...)
				{
				}
			}
		}

		Autodesk::DesignScript::Geometry::Solid^ dynamoSolid = dynamic_cast<Autodesk::DesignScript::Geometry::Solid^>(geometry);
		if (dynamoSolid != nullptr)
		{
			topology = Cell::BySolid(dynamoSolid, tolerance);
		}

		if (topology == nullptr)
		{
			throw gcnew NotImplementedException("This geometry is not currently handled.");
		}

		return topology;
	}

	Topology ^ Topology::ByPolySurface(Autodesk::DesignScript::Geometry::PolySurface ^ polySurface)
	{
		List<Face^>^ pFaces = gcnew List<Face^>();
		array<Autodesk::DesignScript::Geometry::Surface^>^ dynamoSurfaces = polySurface->Surfaces();
		for each(Autodesk::DesignScript::Geometry::Surface^ pDynamoSurface in dynamoSurfaces)
		{
			pFaces->Add(Face::BySurface(pDynamoSurface));
			delete pDynamoSurface;
		}
		return ByFaces(pFaces);
	}

	Topology^ Topology::ByFaces(System::Collections::Generic::IList<Face^>^ faces)
	{
		std::list<TopologicCore::Face::Ptr> coreFaces;
		for each(Face^ pFace in faces)
		{
			coreFaces.push_back(TopologicCore::Topology::Downcast<TopologicCore::Face>(pFace->GetCoreTopologicalQuery()));
		}

		TopologicCore::Topology::Ptr pCoreTopology = nullptr;
		try {
			pCoreTopology = TopologicCore::Topology::ByFaces(coreFaces);
		}
		catch (const std::exception& rkException)
		{
			throw gcnew Exception(gcnew System::String(rkException.what()));
		}

		return Topology::ByCoreTopology(pCoreTopology);
	}

#endif

	//Topology^ Topology::ByVertexIndex(List<array<double, 3>^>^ vertexCoordinates, List<List<int>^>^ vertexIndices)
	//{
	//	throw gcnew System::NotImplementedException();
	//	// TODO: insert return statement here
	//}

	IList<Topology^>^ Topology::ByVerticesIndices(System::Collections::Generic::IList<Vertex^>^ vertices, System::Collections::Generic::IList<System::Collections::Generic::IList<int>^>^ vertexIndices)
	{
		std::vector<TopologicCore::Vertex::Ptr> coreVertices;
		for each(Vertex^ pVertex in vertices)
		{
			TopologicCore::Vertex::Ptr pCoreVertex = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Vertex>(pVertex->GetCoreTopologicalQuery());
			coreVertices.push_back(pCoreVertex);
		}

		std::list<std::list<int>> coreIndices;
		for each(List<int>^ vertex1DIndices in vertexIndices)
		{
			std::list<int> coreVertex1DIndices;
			for each(int vertexIndex in vertex1DIndices)
			{
				if (vertexIndex < 0)
				{
					throw gcnew Exception("The index list contains a negative index.");
				}
				coreVertex1DIndices.push_back(vertexIndex);
			}
			coreIndices.push_back(coreVertex1DIndices);
		}
		std::list<TopologicCore::Topology::Ptr> pCoreTopologies;
		TopologicCore::Topology::ByVertexIndex(coreVertices, coreIndices, pCoreTopologies);

		List<Topology^>^ pTopologies = gcnew List<Topology^>();
		for (const TopologicCore::Topology::Ptr& kpCoreTopology : pCoreTopologies)
		{
			pTopologies->Add(Topology::ByCoreTopology(kpCoreTopology));
		}
		return pTopologies;
	}

	Object^ Topology::Geometry::get()
	{
		return BasicGeometry;
	}

	bool Topology::ExportToBRep(System::String^ path)
	{
		std::shared_ptr<TopologicCore::Topology> pCoreTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		std::string cppPath = msclr::interop::marshal_as<std::string>(path);
		return pCoreTopology->ExportToBRep(cppPath);
	}

	Topology^ Topology::ByImportedBRep(System::String^ filePath)
	{
		std::string cppPath = msclr::interop::marshal_as<std::string>(filePath);
		std::shared_ptr<TopologicCore::Topology> pCoreTopology = TopologicCore::Topology::ByImportedBRep(cppPath);
		Topology^ pTopology = Topology::ByCoreTopology(pCoreTopology);
		return pTopology;
	}

	Topology^ Topology::ByString(System::String^ brepString)
	{
		std::string cppBrepString = msclr::interop::marshal_as<std::string>(brepString);
		std::shared_ptr<TopologicCore::Topology> pCoreTopology = TopologicCore::Topology::ByString(cppBrepString);
		Topology^ pTopology = Topology::ByCoreTopology(pCoreTopology);
		return pTopology;
	}

	String^ Topology::String::get()
	{
		std::shared_ptr<TopologicCore::Topology> pCoreTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		std::string cppBrepString = pCoreTopology->String();
		return gcnew System::String(cppBrepString.c_str());
	}

	String^ Topology::Analyze()
	{
		std::shared_ptr<TopologicCore::Topology> pCoreTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		return gcnew System::String(pCoreTopology->Analyze().c_str());
	}

	bool Topology::IsSame(Topology^ topology)
	{
		std::shared_ptr<TopologicCore::Topology> pCoreTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		std::shared_ptr<TopologicCore::Topology> pOtherCoreTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(topology->GetCoreTopologicalQuery());
		return pCoreTopology->IsSame(pOtherCoreTopology);
	}

	Topology ^ Topology::ClosestSimplestSubshape(Topology^ selector)
	{
		std::shared_ptr<TopologicCore::Topology> pCoreTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		std::shared_ptr<TopologicCore::Topology> pCoreQueryTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(selector->GetCoreTopologicalQuery());

		std::shared_ptr<TopologicCore::Topology> pClosestLowestSubshape = pCoreTopology->ClosestSimplestSubshape(pCoreQueryTopology);
		return ByCoreTopology(pClosestLowestSubshape);
	}

	Topology ^ Topology::SelectSubtopology(Vertex ^ selector, int typeFilter)
	{
		std::shared_ptr<TopologicCore::Topology> pCoreTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		std::shared_ptr<TopologicCore::Vertex> pCoreQueryTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Vertex>(selector->GetCoreTopologicalQuery());

		std::shared_ptr<TopologicCore::Topology> pSelectedSubtopology = pCoreTopology->SelectSubtopology(pCoreQueryTopology, typeFilter);
		return ByCoreTopology(pSelectedSubtopology);
	}

	Topology ^ Topology::ShallowCopy()
	{
		std::shared_ptr<TopologicCore::Topology> pCoreTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		std::shared_ptr<TopologicCore::Topology> pCoreCopyTopology = pCoreTopology->ShallowCopy();
		return Topology::ByCoreTopology(pCoreCopyTopology);

	}

	String^ Topology::TypeAsString::get()
	{
		std::shared_ptr<TopologicCore::Topology> pCoreTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		std::string strType = pCoreTopology->GetTypeAsString();
		return gcnew System::String(strType.c_str());
	}

	int Topology::Type::get()
	{
		std::shared_ptr<TopologicCore::Topology> pCoreTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		TopologicCore::TopologyType topologyType = pCoreTopology->GetType();
		return topologyType;
	}

	Topology ^ Topology::SetDictionary(System::Collections::Generic::Dictionary<System::String^, Object^>^ dictionary)
	{
		try {
			TopologicCore::Topology::Ptr pCoreTopology =
				TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
			TopologicCore::Topology::Ptr pCoreCopyTopology = pCoreTopology->DeepCopy();
			Topology^ copyTopology = Topology::ByCoreTopology(pCoreCopyTopology);
			copyTopology->AddAttributesNoCopy(dictionary);
			return copyTopology;
		}
		catch (const std::exception& e)
		{
			throw gcnew Exception(gcnew System::String(e.what()));
		}
	}

	Topology ^ Topology::SetDictionaries(
		IList<Vertex^>^ selectors,
		IList<System::Collections::Generic::Dictionary<System::String^, Object^>^>^ dictionaries,
		int typeFilter)
	{
		TopologicCore::Topology::Ptr pCoreTopology =
			TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		TopologicCore::Topology::Ptr pCoreCopyTopology = pCoreTopology->DeepCopy();

		std::list<TopologicCore::Vertex::Ptr> coreSelectors;
		for each(Vertex^ selector in selectors)
		{
			TopologicCore::Vertex::Ptr pCoreVertex =
				TopologicCore::TopologicalQuery::Downcast<TopologicCore::Vertex>(selector->GetCoreTopologicalQuery());
			coreSelectors.push_back(pCoreVertex);
		}
		
		std::list<std::map<std::string, TopologicCore::Attribute::Ptr>> coreDictionaries;
		for each(System::Collections::Generic::Dictionary<System::String^, Object^>^ dictionary in dictionaries)
		{
			std::map<std::string, TopologicCore::Attribute::Ptr> coreDictionary;
			for each(KeyValuePair<System::String^, Object^>^ entry in dictionary)
			{
				Attributes::AttributeFactory^ attributeFactory = 
					Attributes::AttributeFactoryManager::Instance->GetFactory(entry->Value);
				Attributes::Attribute^ attribute = attributeFactory->Create(entry->Value);
				std::string cppKey = msclr::interop::marshal_as<std::string>(entry->Key);
				TopologicCore::Attribute::Ptr coreAttribute = attribute->UtilitiesAttribute;
				coreDictionary[cppKey] = coreAttribute;
			}
			coreDictionaries.push_back(coreDictionary);
		}
		try
		{
			TopologicCore::Topology::Ptr pCoreCopyTopologyWithDictionaries = 
				pCoreCopyTopology->SetDictionaries(coreSelectors, coreDictionaries, typeFilter);
			Topology^ copyTopology = Topology::ByCoreTopology(pCoreCopyTopologyWithDictionaries);

			return copyTopology;
		}
		catch (const std::exception& e)
		{
			throw gcnew Exception(gcnew System::String(e.what()));
		}
	}

	Dictionary<String^, Object^>^ Topology::Dictionary::get()
	{
		TopologicCore::Topology::Ptr pCoreTopology =
			TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());

		std::map<std::string, std::shared_ptr<TopologicCore::Attribute>> coreAttributes;
		bool isFound = TopologicCore::AttributeManager::GetInstance().FindAll(pCoreTopology->GetOcctShape(), coreAttributes);
		if (!isFound)
		{
			return nullptr;
		}
		
		System::Collections::Generic::Dictionary<System::String^, Object^>^ dictionary = gcnew System::Collections::Generic::Dictionary<System::String^, Object^>();
		for (const std::pair<std::string, TopologicCore::Attribute::Ptr>& rkAttributePair : coreAttributes)
		{
			System::String^ key = gcnew System::String(rkAttributePair.first.c_str());
			Attributes::AttributeFactory^ attributeFactory = Attributes::AttributeFactoryManager::Instance->GetFactory(rkAttributePair.second);
			dictionary->Add(key, attributeFactory->CreateValue(rkAttributePair.second));
		}

		return dictionary;
	}


	Topology^ Topology::ByCoreTopology(const std::shared_ptr<TopologicCore::Topology>& kpCoreTopology)
	{
		if (kpCoreTopology == nullptr)
		{
			return nullptr;
		}
		System::String^ guid = gcnew System::String(kpCoreTopology->GetInstanceGUID().c_str());
		Factories::TopologyFactory^ topologyFactory = nullptr;
		try {
			topologyFactory = Factories::TopologyFactoryManager::Instance->Find(guid);
		}
		catch (...)
		{
			topologyFactory = Factories::TopologyFactoryManager::Instance->GetDefaultFactory(kpCoreTopology);
		}
		return topologyFactory->Create(TopologicCore::TopologyPtr(kpCoreTopology));
	}

	Object ^ Topology::CleanupGeometryOutput(IList<Object^>^ geometry)
	{
		if (geometry->Count == 1)
		{
			return geometry[0];
		}

		return geometry;
	}

	generic <class T>
		where T: Topology
	T Topology::Copy()
	{
		TopologicCore::Topology::Ptr pCoreTopology =
			TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		TopologicCore::Topology::Ptr pCoreCopyTopology = pCoreTopology->DeepCopy();

		TopologicCore::AttributeManager::GetInstance().DeepCopyAttributes(pCoreTopology->GetOcctShape(), pCoreCopyTopology->GetOcctShape());
		
		Topology^ topology = ByCoreTopology(pCoreCopyTopology);
		return safe_cast<T>(topology);
	}

	IList<Topology^>^ Topology::Filter(IList<Topology^>^ topologies, int typeFilter)
	{
		std::list<TopologicCore::Topology::Ptr> coreTopologies;
		for each(Topology^ topology in topologies)
		{
			TopologicCore::Topology::Ptr pCoreTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(topology->GetCoreTopologicalQuery());
			coreTopologies.push_back(pCoreTopology);
		}

		std::list<TopologicCore::Topology::Ptr> coreFilteredTopologies;
		TopologicCore::Topology::Filter(coreTopologies, typeFilter, coreFilteredTopologies);

		List<Topology^>^ filteredTopologies = gcnew List<Topology^>();
		for (const TopologicCore::Topology::Ptr& kpFilteredTopology : coreFilteredTopologies)
		{
			Topology^ filteredTopology = Topology::ByCoreTopology(kpFilteredTopology);
			filteredTopologies->Add(filteredTopology);
		}
		return filteredTopologies;
	}

	void Topology::RegisterFactory(const TopologicCore::Topology::Ptr & kpCoreTopology, Factories::TopologyFactory^ topologyFactory)
	{
		Factories::TopologyFactoryManager::Instance->Add(kpCoreTopology, topologyFactory);
	}

	void Topology::RegisterFactory(System::String^ guid, Factories::TopologyFactory^ topologyFactory)
	{
		Factories::TopologyFactoryManager::Instance->Add(guid, topologyFactory);
	}
	
	Topology::Topology()
	{
		// Register the factories
		static bool areFactoriesAdded = false;
		if(!areFactoriesAdded)
		{
			RegisterFactory(gcnew System::String(TopologicCore::VertexGUID::Get().c_str()), gcnew Factories::VertexFactory());
			RegisterFactory(gcnew System::String(TopologicCore::EdgeGUID::Get().c_str()), gcnew Factories::EdgeFactory());
			RegisterFactory(gcnew System::String(TopologicCore::WireGUID::Get().c_str()), gcnew Factories::WireFactory());
			RegisterFactory(gcnew System::String(TopologicCore::FaceGUID::Get().c_str()), gcnew Factories::FaceFactory());
			RegisterFactory(gcnew System::String(TopologicCore::ShellGUID::Get().c_str()), gcnew Factories::ShellFactory());
			RegisterFactory(gcnew System::String(TopologicCore::CellGUID::Get().c_str()), gcnew Factories::CellFactory());
			RegisterFactory(gcnew System::String(TopologicCore::CellComplexGUID::Get().c_str()), gcnew Factories::CellComplexFactory());
			RegisterFactory(gcnew System::String(TopologicCore::ClusterGUID::Get().c_str()), gcnew Factories::ClusterFactory());
			RegisterFactory(gcnew System::String(TopologicCore::ApertureGUID::Get().c_str()), gcnew Factories::ApertureFactory());
			areFactoriesAdded = true;
		}
	}

	Topology::~Topology()
	{

	}

	Topology ^ Topology::AddAttributesNoCopy(System::Collections::Generic::Dictionary<System::String^, Object^>^ attributes)
	{
		if (attributes != nullptr)
		{
			TopologicCore::Topology::Ptr pCoreTopology =
				TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());

			for each(KeyValuePair<System::String^, Object^>^ entry in attributes)
			{
				System::Type^ entryValueType = entry->Value->GetType();
				Attributes::AttributeFactoryManager::Instance->SetAttribute(this, entry->Key, entry->Value);
			}
		}
		return this;
	}

	IList<Topology^>^ Topology::Contents::get()
	{
		std::shared_ptr<TopologicCore::Topology> pCoreTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		std::list<std::shared_ptr<TopologicCore::Topology>> coreContents;
		pCoreTopology->Contents(coreContents);

		List<Topology^>^ pTopologies = gcnew List<Topology^>();

		for (const TopologicCore::Topology::Ptr& kpCoreContent : coreContents)
		{
			Topology^ topology = Topology::ByCoreTopology(kpCoreContent);
			pTopologies->Add(topology);
		}
		return pTopologies;
	}

	IList<Aperture^>^ Topology::Apertures::get()
	{
		std::shared_ptr<TopologicCore::Topology> pCoreTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		std::list<std::shared_ptr<TopologicCore::Aperture>> coreApertures;
		pCoreTopology->Apertures(coreApertures);

		List<Aperture^>^ pApertures = gcnew List<Aperture^>();

		for (const TopologicCore::Aperture::Ptr& kpCoreAperture : coreApertures)
		{
			Topology^ topology = Topology::ByCoreTopology(kpCoreAperture);
			Aperture^ aperture = safe_cast<Aperture^>(topology);
			pApertures->Add(aperture);
		}
		return pApertures;
	}

	IList<Topology^>^ Topology::SubContents::get()
	{
		std::shared_ptr<TopologicCore::Topology> pCoreTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		std::list<std::shared_ptr<TopologicCore::Topology>> coreSubContents;
		pCoreTopology->SubContents(coreSubContents);

		List<Topology^>^ pTopologies = gcnew List<Topology^>();

		for (const TopologicCore::Topology::Ptr& kpCoreSubContent : coreSubContents)
		{
			Topology^ topology = Topology::ByCoreTopology(kpCoreSubContent);
			pTopologies->Add(topology);
		}
		return pTopologies;
	}

	IList<Context^>^ Topology::Contexts::get()
	{
		std::shared_ptr<TopologicCore::Topology> pCoreTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		std::list<std::shared_ptr<TopologicCore::Context>> rkCoreContexts;
		pCoreTopology->Contexts(rkCoreContexts);

		List<Context^>^ pContexts = gcnew List<Context^>();

		for (std::list<std::shared_ptr<TopologicCore::Context>>::const_iterator rkCoreContextIterator = rkCoreContexts.cbegin();
			rkCoreContextIterator != rkCoreContexts.cend();
			rkCoreContextIterator++)
		{
			pContexts->Add(gcnew Context(*rkCoreContextIterator));
		}

		return pContexts;
	}

	Topology ^ Topology::AddContent(Topology^ contentTopology, int typeFilter)
	{
		TopologicCore::Topology::Ptr pCoreParentTopology =
			TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());

		TopologicCore::Topology::Ptr pCoreContentTopology =
			TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(contentTopology->GetCoreTopologicalQuery());

		try {
			TopologicCore::Topology::Ptr pCoreCopyParentTopology = pCoreParentTopology->AddContent(pCoreContentTopology, typeFilter);
			return Topology::ByCoreTopology(pCoreCopyParentTopology);
		}
		catch (std::exception& e)
		{
			throw gcnew Exception(gcnew System::String(e.what()));
		}
	}

	Topology ^ Topology::AddContents(IList<Topology^>^ contentTopologies, int typeFilter)
	{
		TopologicCore::Topology::Ptr pCoreParentTopology =
			TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());

		std::list<TopologicCore::Topology::Ptr> coreContentTopologies;
		for each(Topology^ contentTopology in contentTopologies)
		{
			TopologicCore::Topology::Ptr pCoreContentTopology =
					TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(contentTopology->GetCoreTopologicalQuery());
			coreContentTopologies.push_back(pCoreContentTopology);
		}

		try {
			TopologicCore::Topology::Ptr pCoreCopyParentTopology = pCoreParentTopology->AddContents(coreContentTopologies, typeFilter);
			return Topology::ByCoreTopology(pCoreCopyParentTopology);
		}
		catch (std::exception& e)
		{
			throw gcnew Exception(gcnew System::String(e.what()));
		}
	}

	Topology^ Topology::RemoveContent(Topology^ topology)
	{
		throw gcnew NotImplementedException();
	}

	Topology^ Topology::RemoveContents(IList<Topology^>^ contentTopologies)
	{
		TopologicCore::Topology::Ptr pCoreParentTopology =
			TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		std::list<TopologicCore::Topology::Ptr> coreContentTopologies;
		for each(Topology^ contentTopology in contentTopologies)
		{
			TopologicCore::Topology::Ptr pCoreContentTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(contentTopology->GetCoreTopologicalQuery());
			coreContentTopologies.push_back(pCoreContentTopology);
		}

		TopologicCore::Topology::Ptr pCoreNewTopology = pCoreParentTopology->RemoveContents(coreContentTopologies);

		return Topology::ByCoreTopology(pCoreNewTopology);
	}

	Topology ^ Topology::AddApertures(System::Collections::Generic::IList<Topology^>^ apertureTopologies)
	{
		// 1. Copy this topology
		TopologicCore::Topology::Ptr pCoreParentTopology =
			TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		TopologicCore::Topology::Ptr pCoreCopyParentTopology = pCoreParentTopology->DeepCopy();

		// 2. Copy the aperture topology
		for each(Topology^ apertureTopology in apertureTopologies)
		{
			TopologicCore::Topology::Ptr pCoreApertureTopology =
				TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(apertureTopology->GetCoreTopologicalQuery());
			TopologicCore::Topology::Ptr pCoreCopyApertureTopology = pCoreApertureTopology->DeepCopy();
			TopologicCore::Aperture::Ptr pCoreAperture = TopologicCore::Aperture::ByTopologyContext(
				pCoreCopyApertureTopology,
				pCoreCopyParentTopology);
		}
		
		// 3. Return the copy parent topology
		return Topology::ByCoreTopology(pCoreCopyParentTopology);
	}

	IList<Topology^>^ Topology::SharedTopologies(Topology^ topology, int typeFilter)
	{
		std::shared_ptr<TopologicCore::Topology> pCoreTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		std::list<std::shared_ptr<TopologicCore::Topology>> coreSharedTopologies;

		try{
			pCoreTopology->SharedTopologies(TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(topology->GetCoreTopologicalQuery()), typeFilter, coreSharedTopologies);
		}
		catch (std::exception& e)
		{
			throw gcnew Exception(gcnew System::String(e.what()));
		}
		List<Topology^>^ pSharedTopologies = gcnew List<Topology^>();
		for (std::list<std::shared_ptr<TopologicCore::Topology>>::const_iterator kCoreSharedTopologyIterator = coreSharedTopologies.begin();
			kCoreSharedTopologyIterator != coreSharedTopologies.end();
			kCoreSharedTopologyIterator++)
		{
			pSharedTopologies->Add(Topology::ByCoreTopology(*kCoreSharedTopologyIterator));
		}
		return pSharedTopologies;
	}

	Topology^ Topology::Difference(Topology^ topology, bool transferDictionary)
	{
		TopologicCore::Topology::Ptr pCoreTopologyA = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		TopologicCore::Topology::Ptr pCoreTopologyB =
			topology == nullptr ?
			nullptr :
			TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(topology->GetCoreTopologicalQuery());

		try{
			std::shared_ptr<TopologicCore::Topology> pDifferenceCoreTopology = pCoreTopologyA->Difference(pCoreTopologyB, transferDictionary);
			return Topology::ByCoreTopology(pDifferenceCoreTopology);
		}
		catch (std::exception& e)
		{
			throw gcnew Exception(gcnew System::String(e.what()));
		}
	}

	Topology^ Topology::Impose(Topology^ tool, bool transferDictionary)
	{
		TopologicCore::Topology::Ptr pCoreTopologyA = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		TopologicCore::Topology::Ptr pCoreTopologyB =
			tool == nullptr ?
			nullptr :
			TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(tool->GetCoreTopologicalQuery());

		try{
			std::shared_ptr<TopologicCore::Topology> pImposeCoreTopology = pCoreTopologyA->Impose(pCoreTopologyB, transferDictionary);
			return Topology::ByCoreTopology(pImposeCoreTopology);
		}
		catch (std::exception& e)
		{
			throw gcnew Exception(gcnew System::String(e.what()));
		}
	}

	Topology^ Topology::Imprint(Topology^ tool, bool transferDictionary)
	{
		TopologicCore::Topology::Ptr pCoreTopologyA = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		TopologicCore::Topology::Ptr pCoreTopologyB =
			tool == nullptr ?
			nullptr :
			TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(tool->GetCoreTopologicalQuery());

		try{
			std::shared_ptr<TopologicCore::Topology> pImprintCoreTopology = pCoreTopologyA->Imprint(pCoreTopologyB, transferDictionary);
			return Topology::ByCoreTopology(pImprintCoreTopology);
		}
		catch (std::exception& e)
		{
			throw gcnew Exception(gcnew System::String(e.what()));
		}
	}

	Topology^ Topology::Intersect(Topology^ topology, bool transferDictionary)
	{
		TopologicCore::Topology::Ptr pCoreTopologyA = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		TopologicCore::Topology::Ptr pCoreTopologyB =
			topology == nullptr ?
			nullptr :
			TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(topology->GetCoreTopologicalQuery());

		try {
			std::shared_ptr<TopologicCore::Topology> pIntersectionCoreTopology = pCoreTopologyA->Intersect(pCoreTopologyB, transferDictionary);
			return Topology::ByCoreTopology(pIntersectionCoreTopology);
		}
		catch (std::exception& e)
		{
			throw gcnew Exception(gcnew System::String(e.what()));
		}
	}
	
	Topology^ Topology::Union(Topology^ topology, bool transferDictionary)
	{
		TopologicCore::Topology::Ptr pCoreTopologyA = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		TopologicCore::Topology::Ptr pCoreTopologyB =
			topology == nullptr ?
			nullptr :
			TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(topology->GetCoreTopologicalQuery());

		try{
			std::shared_ptr<TopologicCore::Topology> pUnionCoreTopology = pCoreTopologyA->Union(pCoreTopologyB, transferDictionary);
			return Topology::ByCoreTopology(pUnionCoreTopology);
		}
		catch (std::exception& e)
		{
			throw gcnew Exception(gcnew System::String(e.what()));
		}
	}

	Topology^ Topology::SelfMerge()
	{
		TopologicCore::Topology::Ptr pCoreTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());

		try {
			TopologicCore::Topology::Ptr pMergeCoreTopology = pCoreTopology->SelfMerge();
			return Topology::ByCoreTopology(pMergeCoreTopology);
		}
		catch (std::exception& e)
		{
			throw gcnew Exception(gcnew System::String(e.what()));
		}
	}

	Topology^ Topology::Merge(Topology^ topology, bool transferDictionary)
	{
		TopologicCore::Topology::Ptr pCoreTopologyA = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		TopologicCore::Topology::Ptr pCoreTopologyB =
			topology == nullptr ?
			nullptr :
			TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(topology->GetCoreTopologicalQuery());

		try{
			std::shared_ptr<TopologicCore::Topology> pMergeCoreTopology = pCoreTopologyA->Merge(pCoreTopologyB, transferDictionary);
			return Topology::ByCoreTopology(pMergeCoreTopology);
		}
		catch (std::exception& e)
		{
			throw gcnew Exception(gcnew System::String(e.what()));
		}
	}

	Topology^ Topology::Slice(Topology^ tool, bool transferDictionary)
	{
		TopologicCore::Topology::Ptr pCoreTopologyA = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		TopologicCore::Topology::Ptr pCoreTopologyB =
			tool == nullptr ?
			nullptr :
			TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(tool->GetCoreTopologicalQuery());

		try{
			std::shared_ptr<TopologicCore::Topology> pSliceCoreTopology = pCoreTopologyA->Slice(pCoreTopologyB, transferDictionary);
			return Topology::ByCoreTopology(pSliceCoreTopology);
		}
		catch (std::exception& e)
		{
			throw gcnew Exception(gcnew System::String(e.what()));
		}
	}

	Topology ^ Topology::Divide(Topology ^ tool, bool transferDictionary)
	{
		TopologicCore::Topology::Ptr pCoreTopologyA = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		TopologicCore::Topology::Ptr pCoreTopologyB =
			tool == nullptr ?
			nullptr :
			TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(tool->GetCoreTopologicalQuery());

		try {
			std::shared_ptr<TopologicCore::Topology> pSliceCoreTopology = pCoreTopologyA->Divide(pCoreTopologyB, transferDictionary);
			return Topology::ByCoreTopology(pSliceCoreTopology);
		}
		catch (std::exception& e)
		{
			throw gcnew Exception(gcnew System::String(e.what()));
		}
	}

	Topology^ Topology::XOR(Topology^ topology, bool transferDictionary)
	{
		TopologicCore::Topology::Ptr pCoreTopologyA = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		TopologicCore::Topology::Ptr pCoreTopologyB =
			topology == nullptr ?
			nullptr :
			TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(topology->GetCoreTopologicalQuery());

		try{
			std::shared_ptr<TopologicCore::Topology> pSliceCoreTopology = pCoreTopologyA->XOR(pCoreTopologyB, transferDictionary);
			return Topology::ByCoreTopology(pSliceCoreTopology);
		}
		catch (std::exception& e)
		{
			throw gcnew Exception(gcnew System::String(e.what()));
		}
	}

	IList<Topology^>^ Topology::SubTopologies::get()
	{
		std::shared_ptr<TopologicCore::Topology> pCoreTopology = TopologicCore::TopologicalQuery::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());

		std::list<std::shared_ptr<TopologicCore::Topology>> coreTopologies;
		pCoreTopology->SubTopologies(coreTopologies);

		List<Topology^>^ pTopologies = gcnew List<Topology^>();
		for (std::list<std::shared_ptr<TopologicCore::Topology>>::const_iterator kTopologyIterator = coreTopologies.begin();
			kTopologyIterator != coreTopologies.end();
			kTopologyIterator++)
		{
			Topology^ pTopology = Topology::ByCoreTopology(*kTopologyIterator);
			pTopologies->Add(pTopology);
		}
		return pTopologies;
	}

	IList<Shell^>^ Topology::Shells::get()
	{
		TopologicCore::Topology::Ptr pCoreTopology = TopologicCore::Topology::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());

		std::list<TopologicCore::Shell::Ptr> coreShells;
		pCoreTopology->Shells(coreShells);

		List<Shell^>^ pShells = gcnew List<Shell^>();
		for (std::list<TopologicCore::Shell::Ptr>::const_iterator kShellIterator = coreShells.begin();
			kShellIterator != coreShells.end();
			kShellIterator++)
		{
			Shell^ pShell = gcnew Shell(*kShellIterator);
			pShells->Add(pShell);
		}

		return pShells;
	}

	IList<Face^>^ Topology::Faces::get()
	{
		TopologicCore::Topology::Ptr pCoreTopology = TopologicCore::Topology::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());

		std::list<TopologicCore::Face::Ptr> coreFaces;
		pCoreTopology->Faces(coreFaces);

		List<Face^>^ pFaces = gcnew List<Face^>();
		for (std::list<TopologicCore::Face::Ptr>::const_iterator kFaceIterator = coreFaces.begin();
			kFaceIterator != coreFaces.end();
			kFaceIterator++)
		{
			Face^ pFace = gcnew Face(*kFaceIterator);
			pFaces->Add(pFace);
		}

		return pFaces;
	}

	IList<Wire^>^ Topology::Wires::get()
	{
		TopologicCore::Topology::Ptr pCoreTopology = TopologicCore::Topology::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());

		std::list<TopologicCore::Wire::Ptr> coreWires;
		pCoreTopology->Wires(coreWires);

		List<Wire^>^ pWires = gcnew List<Wire^>();
		for (std::list<TopologicCore::Wire::Ptr>::const_iterator kWireIterator = coreWires.begin();
			kWireIterator != coreWires.end();
			kWireIterator++)
		{
			Wire^ pWire = gcnew Wire(*kWireIterator);
			pWires->Add(pWire);
		}

		return pWires;
	}

	IList<Edge^>^ Topology::Edges::get()
	{
		TopologicCore::Topology::Ptr pCoreTopology = TopologicCore::Topology::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());

		std::list<TopologicCore::Edge::Ptr> coreEdges;
		pCoreTopology->Edges(coreEdges);

		List<Edge^>^ pEdges = gcnew List<Edge^>();
		for (std::list<TopologicCore::Edge::Ptr>::const_iterator kEdgeIterator = coreEdges.begin();
			kEdgeIterator != coreEdges.end();
			kEdgeIterator++)
		{
			Edge^ pEdge = gcnew Edge(*kEdgeIterator);
			pEdges->Add(pEdge);
		}

		return pEdges;
	}

	IList<Vertex^>^ Topology::Vertices::get()
	{
		TopologicCore::Topology::Ptr pCoreTopology = TopologicCore::Topology::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());

		std::list<TopologicCore::Vertex::Ptr> coreVertices;
		pCoreTopology->Vertices(coreVertices);

		List<Vertex^>^ pVertices = gcnew List<Vertex^>();
		for (std::list<TopologicCore::Vertex::Ptr>::const_iterator kVertexIterator = coreVertices.begin();
			kVertexIterator != coreVertices.end();
			kVertexIterator++)
		{
			Vertex^ pVertex = gcnew Vertex(*kVertexIterator);
			pVertices->Add(pVertex);
		}

		return pVertices;
	}

	IList<Cell^>^ Topology::Cells::get()
	{
		TopologicCore::Topology::Ptr pCoreTopology = TopologicCore::Topology::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());

		std::list<TopologicCore::Cell::Ptr> coreCells;
		pCoreTopology->Cells(coreCells);

		List<Cell^>^ pCells = gcnew List<Cell^>();
		for (std::list<TopologicCore::Cell::Ptr>::const_iterator kCellIterator = coreCells.begin();
			kCellIterator != coreCells.end();
			kCellIterator++)
		{
			Cell^ pCell = gcnew Cell(*kCellIterator);
			pCells->Add(pCell);
		}

		return pCells;
	}

	IList<CellComplex^>^ Topology::CellComplexes::get()
	{
		TopologicCore::Topology::Ptr pCoreTopology = TopologicCore::Topology::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());

		std::list<TopologicCore::CellComplex::Ptr> coreCellComplexes;
		pCoreTopology->CellComplexes(coreCellComplexes);

		List<CellComplex^>^ pCellComplexes = gcnew List<CellComplex^>();
		for (std::list<TopologicCore::CellComplex::Ptr>::const_iterator kCellComplexIterator = coreCellComplexes.begin();
			kCellComplexIterator != coreCellComplexes.end();
			kCellComplexIterator++)
		{
			CellComplex^ pCellComplex = gcnew CellComplex(*kCellComplexIterator);
			pCellComplexes->Add(pCellComplex);
		}

		return pCellComplexes;
	}

	Vertex^ Topology::CenterOfMass::get()
	{
		TopologicCore::Topology::Ptr pCoreTopology = TopologicCore::Topology::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		TopologicCore::Vertex::Ptr pCoreCenterOfMass = pCoreTopology->CenterOfMass();
		return gcnew Vertex(pCoreCenterOfMass);
	}

	Vertex^ Topology::Centroid::get()
	{
		TopologicCore::Topology::Ptr pCoreTopology = TopologicCore::Topology::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());
		TopologicCore::Vertex::Ptr pCoreCentroid = pCoreTopology->Centroid();
		return gcnew Vertex(pCoreCentroid);
	}

	bool Topology::IsReversed::get()
	{
		TopologicCore::Topology::Ptr pCoreTopology = TopologicCore::Topology::Downcast<TopologicCore::Topology>(GetCoreTopologicalQuery());

		return pCoreTopology->IsReversed();
	}
}