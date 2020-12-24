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

#include "NurbsCurve.h"
#include "Vertex.h"

namespace Topologic
{
	NurbsCurve::NurbsCurve(const std::shared_ptr<TopologicCore::NurbsCurve>& kpCoreNurbsCurve)
		: m_pCoreNurbsCurve(kpCoreNurbsCurve != nullptr ? new TopologicCore::NurbsCurve::Ptr(kpCoreNurbsCurve) : throw gcnew Exception("A null Nurbs Curve was created."))
	{

	}

	NurbsCurve::~NurbsCurve()
	{

	}

	bool NurbsCurve::IsPeriodic::get()
	{
		return (*m_pCoreNurbsCurve)->IsPeriodic();
	}

	bool NurbsCurve::IsRational::get()
	{
		return (*m_pCoreNurbsCurve)->IsRational();
	}

	int NurbsCurve::Degree::get()
	{
		return (*m_pCoreNurbsCurve)->Degree();
	}

	List<Vertex^>^ NurbsCurve::ControlVertices::get()
	{
		std::list<TopologicCore::Vertex::Ptr> coreControlVertices;
		(*m_pCoreNurbsCurve)->ControlVertices(coreControlVertices);

		List<Vertex^>^ controlVertices = gcnew List<Vertex^>();
		for (const TopologicCore::Vertex::Ptr kpCoreControlVertex : coreControlVertices)
		{
			Vertex^ vertex = safe_cast<Vertex^>(Topology::ByCoreTopology(kpCoreControlVertex));
			controlVertices->Add(vertex);
		}
		return controlVertices;
	}

	List<double>^ NurbsCurve::Knots::get()
	{
		std::list<double> coreKnots;
		(*m_pCoreNurbsCurve)->Knots(coreKnots);

		List<double>^ knots = gcnew List<double>();
		for (const double kKnot : coreKnots)
		{
			knots->Add(kKnot);
		}
		return knots;
	}

	double NurbsCurve::FirstParameter::get()
	{
		return (*m_pCoreNurbsCurve)->FirstParameter();
	}

	double NurbsCurve::LastParameter::get()
	{
		return (*m_pCoreNurbsCurve)->LastParameter();
	}
}