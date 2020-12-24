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

#include "Cell.h"
#include <Vertex.h>
#include <Edge.h>
#include <Wire.h>
#include <Face.h>
#include <Shell.h>
#include <CellComplex.h>

#include <TopologicCore/include/Utilities/CellUtility.h>
#include <TopologicCore/include/Utilities/TopologyUtility.h>

#include <gp_Quaternion.hxx>

#include <assert.h>

namespace Topologic
{
	Cell^ Cell::ByFaces(System::Collections::Generic::IList<Face^>^ faces, double tolerance)
	{
		std::list<TopologicCore::Face::Ptr> coreFaces;
		for each(Face^ pFace in faces)
		{
			coreFaces.push_back(TopologicCore::Topology::Downcast<TopologicCore::Face>(pFace->GetCoreTopologicalQuery()));
		}

		TopologicCore::Cell::Ptr pCoreCell = nullptr;
		try {
			pCoreCell = TopologicCore::Cell::ByFaces(coreFaces, tolerance);
		}
		catch (const std::exception& rkException)
		{
			throw gcnew Exception(gcnew System::String(rkException.what()));
		}

		return gcnew Cell(pCoreCell);
	}

	Cell^ Cell::ByShell(Shell^ shell)
	{
		try {
			TopologicCore::Cell::Ptr pCoreCell = TopologicCore::Cell::ByShell(TopologicCore::Topology::Downcast<TopologicCore::Shell>(shell->GetCoreTopologicalQuery()));
			return gcnew Cell(pCoreCell);
		}
		catch (const std::exception& rkException)
		{
			throw gcnew Exception(gcnew System::String(rkException.what()));
		}
	}

	IList<CellComplex^>^ Cell::CellComplexes::get()
	{
		TopologicCore::Cell::Ptr pCoreCell = TopologicCore::Topology::Downcast<TopologicCore::Cell>(GetCoreTopologicalQuery());
		std::list<TopologicCore::CellComplex::Ptr> coreCellComplexes;
		pCoreCell->CellComplexes(coreCellComplexes);

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

	IList<Shell^>^ Cell::Shells::get()
	{
		TopologicCore::Cell::Ptr pCoreCell = TopologicCore::Topology::Downcast<TopologicCore::Cell>(GetCoreTopologicalQuery());

		std::list<TopologicCore::Shell::Ptr> coreShells;
		pCoreCell->Shells(coreShells);

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

	IList<Face^>^ Cell::Faces::get()
	{
		TopologicCore::Cell::Ptr pCoreCell = TopologicCore::Topology::Downcast<TopologicCore::Cell>(GetCoreTopologicalQuery());

		std::list<TopologicCore::Face::Ptr> coreFaces;
		pCoreCell->Faces(coreFaces);

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

	IList<Wire^>^ Cell::Wires::get()
	{
		TopologicCore::Cell::Ptr pCoreCell = TopologicCore::Topology::Downcast<TopologicCore::Cell>(GetCoreTopologicalQuery());

		std::list<TopologicCore::Wire::Ptr> coreWires;
		pCoreCell->Wires(coreWires);

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

	IList<Edge^>^ Cell::Edges::get()
	{
		TopologicCore::Cell::Ptr pCoreCell = TopologicCore::Topology::Downcast<TopologicCore::Cell>(GetCoreTopologicalQuery());

		std::list<TopologicCore::Edge::Ptr> coreEdges;
		pCoreCell->Edges(coreEdges);

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

	IList<Vertex^>^ Cell::Vertices::get()
	{
		TopologicCore::Cell::Ptr pCoreCell = TopologicCore::Topology::Downcast<TopologicCore::Cell>(GetCoreTopologicalQuery());

		std::list<TopologicCore::Vertex::Ptr> coreVertices;
		pCoreCell->Vertices(coreVertices);

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

	IList<Cell^>^ Cell::AdjacentCells::get()
	{
		TopologicCore::Cell::Ptr pCoreCell = TopologicCore::Topology::Downcast<TopologicCore::Cell>(GetCoreTopologicalQuery());
		std::list<TopologicCore::Cell::Ptr> coreAdjacentCells;
		try {
			pCoreCell->AdjacentCells(coreAdjacentCells);
		}
		catch (const std::exception& rkException)
		{
			throw gcnew Exception(gcnew System::String(rkException.what()));
		}

		List<Cell^>^ pAdjacentCells = gcnew List<Cell^>();
		for (std::list<TopologicCore::Cell::Ptr>::const_iterator kCellIterator = coreAdjacentCells.begin();
			kCellIterator != coreAdjacentCells.end();
			kCellIterator++)
		{
			Cell^ pCell = gcnew Cell(*kCellIterator);
			pAdjacentCells->Add(pCell);
		}

		return pAdjacentCells;
	}

	IList<Face^>^ Cell::SharedFaces(Cell^ cell)
	{
		TopologicCore::Cell::Ptr pCoreCell1 = TopologicCore::Topology::Downcast<TopologicCore::Cell>(GetCoreTopologicalQuery());
		TopologicCore::Cell::Ptr pCoreCell2 = TopologicCore::Topology::Downcast<TopologicCore::Cell>(cell->GetCoreTopologicalQuery());

		std::list<TopologicCore::Face::Ptr> sharedFaces;
		pCoreCell1->SharedFaces(pCoreCell2, sharedFaces);

		List<Face^>^ pSharedFaces = gcnew List<Face^>();
		for(std::list<TopologicCore::Face::Ptr>::const_iterator kFaceIterator = sharedFaces.begin();
			kFaceIterator != sharedFaces.end();
			kFaceIterator++)
		{
			Face^ pFace = gcnew Face(*kFaceIterator);
			pSharedFaces->Add(pFace);
		}

		return pSharedFaces;
	}

	IList<Edge^>^ Cell::SharedEdges(Cell^ cell)
	{
		TopologicCore::Cell::Ptr pCoreCell1 = TopologicCore::Topology::Downcast<TopologicCore::Cell>(GetCoreTopologicalQuery());
		TopologicCore::Cell::Ptr pCoreCell2 = TopologicCore::Topology::Downcast<TopologicCore::Cell>(cell->GetCoreTopologicalQuery());

		std::list<TopologicCore::Edge::Ptr> sharedEdges;
		pCoreCell1->SharedEdges(pCoreCell2, sharedEdges);

		List<Edge^>^ pSharedEdges = gcnew List<Edge^>();
		for (std::list<TopologicCore::Edge::Ptr>::const_iterator kEdgeIterator = sharedEdges.begin();
			kEdgeIterator != sharedEdges.end();
			kEdgeIterator++)
		{
			Edge^ pEdge = gcnew Edge(*kEdgeIterator);
			pSharedEdges->Add(pEdge);
		}

		return pSharedEdges;
	}

	IList<Vertex^>^ Cell::SharedVertices(Cell^ cell)
	{
		TopologicCore::Cell::Ptr pCoreCell1 = TopologicCore::Topology::Downcast<TopologicCore::Cell>(GetCoreTopologicalQuery());
		TopologicCore::Cell::Ptr pCoreCell2 = TopologicCore::Topology::Downcast<TopologicCore::Cell>(cell->GetCoreTopologicalQuery());

		std::list<TopologicCore::Vertex::Ptr> sharedVertices;
		pCoreCell1->SharedVertices(pCoreCell2, sharedVertices);

		List<Vertex^>^ pSharedVertices = gcnew List<Vertex^>();
		for (std::list<TopologicCore::Vertex::Ptr>::const_iterator kVertexIterator = sharedVertices.begin();
			kVertexIterator != sharedVertices.end();
			kVertexIterator++)
		{
			Vertex^ pVertex = gcnew Vertex(*kVertexIterator);
			pSharedVertices->Add(pVertex);
		}

		return pSharedVertices;
	}

	Shell^ Cell::ExternalBoundary::get()
	{
		TopologicCore::Cell::Ptr pCoreCell = TopologicCore::Topology::Downcast<TopologicCore::Cell>(GetCoreTopologicalQuery());
		return gcnew Shell(pCoreCell->ExternalBoundary());
	}

	IList<Shell^>^ Cell::InternalBoundaries::get()
	{
		TopologicCore::Cell::Ptr pCoreCell = TopologicCore::Topology::Downcast<TopologicCore::Cell>(GetCoreTopologicalQuery());
		std::list<TopologicCore::Shell::Ptr> coreInnerShells;
		pCoreCell->InternalBoundaries(coreInnerShells);

		List<Shell^>^ pInnerShells = gcnew List<Shell^>();
		for (std::list<TopologicCore::Shell::Ptr>::const_iterator kShellIterator = coreInnerShells.begin();
			kShellIterator != coreInnerShells.end();
			kShellIterator++)
		{
			Shell^ pShell = gcnew Shell(*kShellIterator);
			pInnerShells->Add(pShell);
		}

		return pInnerShells;
	}

	Object^ Cell::BasicGeometry::get()
	{
#ifdef TOPOLOGIC_DYNAMO
		List<Autodesk::DesignScript::Geometry::Surface^>^ pDynamoSurfaces = gcnew List<Autodesk::DesignScript::Geometry::Surface^>();
		List<Object^>^ pDynamoGeometries = gcnew List<Object^>();
		IList<Face^>^ pFaces = Faces;
		bool hasFallbackVisualization = false;
		for each(Face^ pFace in pFaces)
		{
			Object^ pFaceGeometry = pFace->BasicGeometry;
			Autodesk::DesignScript::Geometry::Surface^ pDynamoSurface = dynamic_cast<Autodesk::DesignScript::Geometry::Surface^>(pFaceGeometry);
			pDynamoGeometries->Add(pFaceGeometry);
			if(pDynamoSurface != nullptr)
			{
				pDynamoSurfaces->Add(pDynamoSurface);
			}
			else
			{
				// We are getting a null geometry! This is possible when the face gives a Mesh (i.e. not a Surface) as its geometry representation.
				hasFallbackVisualization = true;
			}
		}

		if(!hasFallbackVisualization)
		{
			try {
				if (!pDynamoSurfaces->Contains(nullptr))
				{
					Autodesk::DesignScript::Geometry::Solid^ pDynamoSolid = Autodesk::DesignScript::Geometry::Solid::ByJoinedSurfaces(pDynamoSurfaces);
					for each(Object^ pDynamoGeometry in pDynamoGeometries)
					{
						delete pDynamoGeometry;
					}
					return pDynamoSolid;
				}
				else
				{
					return pDynamoSurfaces;
				}
			}
			catch (Exception^)
			{

			}
		}

		return pDynamoGeometries;
#else
		return nullptr;
#endif
	}

#ifdef TOPOLOGIC_DYNAMO
	Cell^ Cell::BySolid(Autodesk::DesignScript::Geometry::Solid^ solid, double tolerance)
	{
		if (solid->GetType() == Autodesk::DesignScript::Geometry::Sphere::typeid)
		{
			return BySphere(safe_cast<Autodesk::DesignScript::Geometry::Sphere^>(solid));
		}
		else if (solid->GetType() == Autodesk::DesignScript::Geometry::Cuboid::typeid)
		{
			return ByCuboid(safe_cast<Autodesk::DesignScript::Geometry::Cuboid^>(solid));
		}
		else if (solid->GetType() == Autodesk::DesignScript::Geometry::Cylinder::typeid)
		{
			return ByCylinder(safe_cast<Autodesk::DesignScript::Geometry::Cylinder^>(solid));
		}
		else if (solid->GetType() == Autodesk::DesignScript::Geometry::Cone::typeid)
		{
			return ByCone(safe_cast<Autodesk::DesignScript::Geometry::Cone^>(solid));
		}
		else
		{
			Autodesk::DesignScript::Geometry::PolySurface^ pDynamoPolysurface =
				Autodesk::DesignScript::Geometry::PolySurface::BySolid(solid);
			List<Face^>^ pFaces = gcnew List<Face^>();
			array<Autodesk::DesignScript::Geometry::Surface^>^ dynamoSurfaces = pDynamoPolysurface->Surfaces();
			for each(Autodesk::DesignScript::Geometry::Surface^ pDynamoSurface in dynamoSurfaces)
			{
				pFaces->Add(Face::BySurface(pDynamoSurface));
				delete pDynamoSurface;
			}
			delete pDynamoPolysurface;
			return ByFaces(pFaces, tolerance);
		}
	}

	Cell ^ Cell::BySphere(Autodesk::DesignScript::Geometry::Sphere ^ sphere)
	{
		TopologicCore::Cell::Ptr pCoreSphere = TopologicUtilities::CellUtility::BySphere(
			sphere->CenterPoint->X,
			sphere->CenterPoint->Y, 
			sphere->CenterPoint->Z, 
			sphere->Radius);
		return gcnew Cell(pCoreSphere);
	}

	Cell ^ Cell::ByCylinder(Autodesk::DesignScript::Geometry::Cylinder ^ cylinder)
	{
		TopologicCore::Cell::Ptr pCoreCylinder = TopologicUtilities::CellUtility::ByCylinder(
			cylinder->StartPoint->X,
			cylinder->StartPoint->Y,
			cylinder->StartPoint->Z,
			cylinder->Axis->X,
			cylinder->Axis->Y,
			cylinder->Axis->Z,
			cylinder->Radius,
			cylinder->StartPoint->DistanceTo(cylinder->EndPoint));
		return gcnew Cell(pCoreCylinder);
	}

	Cell ^ Cell::ByCone(Autodesk::DesignScript::Geometry::Cone ^ cone)
	{
		// Open CASCADE always makes an upright cone. Dynamo can make a cone of any orientation
		// Create the quaternion rotation first
		gp_Vec occtUprightNormal(
			cone->ContextCoordinateSystem->ZAxis->X,
			cone->ContextCoordinateSystem->ZAxis->Y,
			cone->ContextCoordinateSystem->ZAxis->Z);
		gp_Vec occtUprightXAxis(
			cone->ContextCoordinateSystem->XAxis->X,
			cone->ContextCoordinateSystem->XAxis->Y,
			cone->ContextCoordinateSystem->XAxis->Z);
		gp_Pnt occtStartPoint(cone->StartPoint->X, cone->StartPoint->Y, cone->StartPoint->Z);
		gp_Pnt occtEndPoint(cone->EndPoint->X, cone->EndPoint->Y, cone->EndPoint->Z);
		gp_Vec occtOrientedNormal(occtStartPoint, occtEndPoint);
		gp_Quaternion occtQuaternion(occtUprightNormal, occtOrientedNormal);

		// Rotate the upright normal and x axis
		gp_Vec occtRotatedUprightNormal = occtQuaternion * occtUprightNormal;
		gp_Vec occtRotatedUprightXAxis = occtQuaternion * occtUprightXAxis;

		// Create the cone
		TopologicCore::Cell::Ptr pCoreCone = nullptr;
		try {
			pCoreCone = TopologicUtilities::CellUtility::ByCone(
				cone->ContextCoordinateSystem->Origin->X,
				cone->ContextCoordinateSystem->Origin->Y,
				cone->ContextCoordinateSystem->Origin->Z,
				occtRotatedUprightNormal.X(),
				occtRotatedUprightNormal.Y(),
				occtRotatedUprightNormal.Z(),
				occtRotatedUprightXAxis.X(),
				occtRotatedUprightXAxis.Y(),
				occtRotatedUprightXAxis.Z(),
				cone->StartRadius,
				cone->EndRadius,
				cone->StartPoint->DistanceTo(cone->EndPoint));
		}
		catch (const std::exception& rkException)
		{
			throw gcnew Exception(gcnew System::String(rkException.what()));
		}


		// Translate to new start point
		TopologicCore::Cell::Ptr pCoreTranslatedCone = 
			TopologicCore::TopologicalQuery::Downcast<TopologicCore::Cell>(
				TopologicUtilities::TopologyUtility::Translate(
				pCoreCone,
				cone->StartPoint->X - cone->ContextCoordinateSystem->Origin->X,
				cone->StartPoint->Y - cone->ContextCoordinateSystem->Origin->Y,
				cone->StartPoint->Z - cone->ContextCoordinateSystem->Origin->Z));

		return gcnew Cell(pCoreTranslatedCone);
	}

	Cell^ Cell::ByCuboid(Autodesk::DesignScript::Geometry::Cuboid^ cuboid)
	{
		Autodesk::DesignScript::Geometry::Point^ pDynamoCentroid = cuboid->Centroid();
		double length = cuboid->Length;
		double width = cuboid->Width;
		double height = cuboid->Height;

		Autodesk::DesignScript::Geometry::CoordinateSystem^ pDynamoCoordinateSystem = cuboid->ContextCoordinateSystem;

		// v1.0: Does not handle custom transformation.
		double xScale = pDynamoCoordinateSystem->XScaleFactor;
		double yScale = pDynamoCoordinateSystem->YScaleFactor;
		double zScale = pDynamoCoordinateSystem->ZScaleFactor;
		double oneMinusPrecision = 1.0 - 0.0001;
		double onePlusPrecision = 1.0 + 0.0001; 
		
		TopologicCore::Cell::Ptr pCoreCell = nullptr;
		pCoreCell = TopologicUtilities::CellUtility::ByCuboid(
			pDynamoCentroid->X,
			pDynamoCentroid->Y,
			pDynamoCentroid->Z,
			width,
			length,
			height,
			pDynamoCoordinateSystem->Determinant * pDynamoCoordinateSystem->ZAxis->X,
			pDynamoCoordinateSystem->Determinant * pDynamoCoordinateSystem->ZAxis->Y,
			pDynamoCoordinateSystem->Determinant * pDynamoCoordinateSystem->ZAxis->Z,
			pDynamoCoordinateSystem->Determinant * pDynamoCoordinateSystem->XAxis->X,
			pDynamoCoordinateSystem->Determinant * pDynamoCoordinateSystem->XAxis->Y,
			pDynamoCoordinateSystem->Determinant * pDynamoCoordinateSystem->XAxis->Z,
			pDynamoCoordinateSystem->Determinant * pDynamoCoordinateSystem->YAxis->X,
			pDynamoCoordinateSystem->Determinant * pDynamoCoordinateSystem->YAxis->Y,
			pDynamoCoordinateSystem->Determinant * pDynamoCoordinateSystem->YAxis->Z
		);

		TopologicCore::Vertex::Ptr coreCentroid = pCoreCell->CenterOfMass();

		delete pDynamoCentroid;
		delete pDynamoCoordinateSystem;
		return gcnew Cell(pCoreCell);
	}
#endif

	std::shared_ptr<TopologicCore::TopologicalQuery> Cell::GetCoreTopologicalQuery()
	{
		assert(m_pCoreCell != nullptr && "Cell::m_pCoreCell is null.");
		if (m_pCoreCell == nullptr)
		{
			throw gcnew Exception("Cell::m_pCoreCell is null.");
		}

		return *m_pCoreCell;
	}

	int Cell::Type()
	{
		return TopologicCore::Cell::Type();
	}

	Cell::Cell()
		: Topologic::Topology()
		, m_pCoreCell(nullptr)
	{

	}

	Cell::Cell(const TopologicCore::Cell::Ptr& kpCoreCell)
		: Topology()
		, m_pCoreCell(kpCoreCell != nullptr ? new TopologicCore::Cell::Ptr(kpCoreCell) : throw gcnew Exception("A null cell was created."))
	{

	}

	Cell::~Cell()
	{
		delete m_pCoreCell;
	}
}