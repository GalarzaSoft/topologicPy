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

#include "FaceUtility.h"
#include "Vertex.h"
#include "Wire.h"
#include "Shell.h"
#include "Cell.h"

#include <TopologicCore/include/Utilities/FaceUtility.h>

namespace Topologic {
	namespace Utilities {
		double FaceUtility::Area(Face^ face)
		{
			TopologicCore::Face::Ptr pCoreFace = TopologicCore::Topology::Downcast<TopologicCore::Face>(face->GetCoreTopologicalQuery());
			return TopologicUtilities::FaceUtility::Area(pCoreFace);
		}

		Face ^ FaceUtility::ByVertices(System::Collections::Generic::IList<System::Collections::Generic::IList<Vertex^>^>^ vertices)
		{
			std::list<std::list<TopologicCore::Vertex::Ptr>> coreVertices;
			for each(System::Collections::Generic::IList<Vertex^>^ verticesList in vertices)
			{
				std::list<TopologicCore::Vertex::Ptr> coreVerticesList;
				for each(Vertex^ vertex in verticesList)
				{
					coreVerticesList.push_back(TopologicCore::Topology::Downcast<TopologicCore::Vertex>(vertex->GetCoreTopologicalQuery()));
				}
				coreVertices.push_back(coreVerticesList);
			}
			try {
				TopologicCore::Face::Ptr pCoreFace = TopologicUtilities::FaceUtility::ByVertices(coreVertices);
				return gcnew Face(pCoreFace);
			}
			catch (const std::exception& e)
			{
				throw gcnew Exception(gcnew String(e.what()));
			}
		}

		IList<double>^ FaceUtility::ParametersAtVertex(Face^ face, Vertex^ vertex)
		{
			TopologicCore::Face::Ptr pCoreFace = TopologicCore::Topology::Downcast<TopologicCore::Face>(face->GetCoreTopologicalQuery());
			double u = 0.0, v = 0.0;
			TopologicUtilities::FaceUtility::ParametersAtVertex(
				pCoreFace,
				TopologicCore::Topology::Downcast<TopologicCore::Vertex>(vertex->GetCoreTopologicalQuery()),
				u, v
			);

			List<double>^ uv = gcnew List<double>();
			uv->Add(u);
			uv->Add(v);
			return uv;
		}

		IList<double>^ FaceUtility::NormalAtParameters(Face^ face, double u, double v)
		{
			TopologicCore::Face::Ptr pCoreFace = TopologicCore::Topology::Downcast<TopologicCore::Face>(face->GetCoreTopologicalQuery());
			gp_Dir normal = TopologicUtilities::FaceUtility::NormalAtParameters(pCoreFace, u, v);

			List<double>^ netNormal = gcnew List<double>();
			netNormal->Add(normal.X());
			netNormal->Add(normal.Y());
			netNormal->Add(normal.Z());
			return netNormal;
		}

		Vertex^ FaceUtility::VertexAtParameters(Face^ face, double u, double v)
		{
			TopologicCore::Face::Ptr pCoreFace = TopologicCore::Topology::Downcast<TopologicCore::Face>(face->GetCoreTopologicalQuery());
			try {
				TopologicCore::Vertex::Ptr pCoreVertex = TopologicUtilities::FaceUtility::VertexAtParameters(
					pCoreFace, u, v);
				return safe_cast<Vertex^>(Topology::ByCoreTopology(pCoreVertex));
			}
			catch (std::exception& e)
			{
				throw gcnew Exception(gcnew String(e.what()));
			}
			return nullptr;
		}

		Face^ FaceUtility::TrimByWire(Face^ face, Wire^ wire, bool reverseWire)
		{
			TopologicCore::Face::Ptr pCoreFace = TopologicCore::Topology::Downcast<TopologicCore::Face>(face->GetCoreTopologicalQuery());
			TopologicCore::Wire::Ptr pCoreWire = TopologicCore::Topology::Downcast<TopologicCore::Wire>(wire->GetCoreTopologicalQuery());

			TopologicCore::Face::Ptr pTrimmedFace = TopologicUtilities::FaceUtility::TrimByWire(pCoreFace, pCoreWire, reverseWire);
			return safe_cast<Face^>(Topology::ByCoreTopology(pTrimmedFace));
		}

		IList<Face^>^ FaceUtility::Triangulate(Face ^ face, double deflection)
		{
			TopologicCore::Face::Ptr pCoreFace = TopologicCore::Topology::Downcast<TopologicCore::Face>(face->GetCoreTopologicalQuery());
			std::list<TopologicCore::Face::Ptr> triangulation;
			TopologicUtilities::FaceUtility::Triangulate(pCoreFace, deflection, triangulation);
			
			List<Face^>^ faces = gcnew List<Face^>();
			for (const TopologicCore::Face::Ptr& kpFace : triangulation)
			{
				faces->Add(safe_cast<Face^>(Topology::ByCoreTopology(kpFace)));
			}

			return faces;
		}

		IList<Shell^>^ FaceUtility::AdjacentShells(Face ^ face, Topology ^ parentTopology)
		{
			TopologicCore::Face::Ptr pCoreFace = TopologicCore::Topology::Downcast<TopologicCore::Face>(face->GetCoreTopologicalQuery());
			TopologicCore::Topology::Ptr pCoreParentTopology = TopologicCore::Topology::Downcast<TopologicCore::Topology>(parentTopology->GetCoreTopologicalQuery());

			std::list<TopologicCore::Shell::Ptr> coreAdjacentShells;
			try {
				TopologicUtilities::FaceUtility::AdjacentShells(pCoreFace, pCoreParentTopology, coreAdjacentShells);
			}
			catch (const std::exception& rkException)
			{
				throw gcnew Exception(gcnew String(rkException.what()));
			}

			List<Shell^>^ adjacentShells = gcnew List<Shell^>();
			for (std::list<TopologicCore::Shell::Ptr>::const_iterator kAdjacentShellIterator = coreAdjacentShells.begin();
				kAdjacentShellIterator != coreAdjacentShells.end();
				kAdjacentShellIterator++)
			{
				Shell^ shell = gcnew Shell(*kAdjacentShellIterator);
				adjacentShells->Add(shell);
			}

			return adjacentShells;
		}

		IList<Cell^>^ FaceUtility::AdjacentCells(Face ^ face, Topology ^ parentTopology)
		{
			TopologicCore::Face::Ptr pCoreFace = TopologicCore::Topology::Downcast<TopologicCore::Face>(face->GetCoreTopologicalQuery());
			TopologicCore::Topology::Ptr pCoreParentTopology = TopologicCore::Topology::Downcast<TopologicCore::Topology>(parentTopology->GetCoreTopologicalQuery());

			std::list<TopologicCore::Cell::Ptr> coreAdjacentCells;
			try {
				TopologicUtilities::FaceUtility::AdjacentCells(pCoreFace, pCoreParentTopology, coreAdjacentCells);
			}
			catch (const std::exception& rkException)
			{
				throw gcnew Exception(gcnew String(rkException.what()));
			}

			List<Cell^>^ adjacentCells = gcnew List<Cell^>();
			for (std::list<TopologicCore::Cell::Ptr>::const_iterator kAdjacentCellIterator = coreAdjacentCells.begin();
				kAdjacentCellIterator != coreAdjacentCells.end();
				kAdjacentCellIterator++)
			{
				Cell^ cell = gcnew Cell(*kAdjacentCellIterator);
				adjacentCells->Add(cell);
			}

			return adjacentCells;
		}

		Vertex^ FaceUtility::InternalVertex(Face^ face, double tolerance)
		{
			TopologicCore::Face::Ptr pCoreFace = TopologicCore::Topology::Downcast<TopologicCore::Face>(face->GetCoreTopologicalQuery());
			try
			{
				TopologicCore::Vertex::Ptr pCoreVertex = TopologicUtilities::FaceUtility::InternalVertex(pCoreFace, tolerance);
				return gcnew Vertex(pCoreVertex);
			}
			catch (const std::exception& rkException)
			{
				throw gcnew Exception(gcnew String(rkException.what()));
			}
		}

		bool FaceUtility::IsInside(Face^ face, Vertex ^ vertex, double tolerance)
		{
			TopologicCore::Face::Ptr pCoreFace = TopologicCore::Topology::Downcast<TopologicCore::Face>(face->GetCoreTopologicalQuery());
			TopologicCore::Vertex::Ptr pCoreVertex = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(vertex->GetCoreTopologicalQuery());
			return TopologicUtilities::FaceUtility::IsInside(pCoreFace, pCoreVertex, tolerance);
		}

	}
}