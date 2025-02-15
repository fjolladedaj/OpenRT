#include "ShaderBlinn.h"
#include "Scene.h"
#include "Ray.h"

namespace rt {
	Vec3f CShaderBlinn::shade(const Ray& ray) const
	{
		Vec3f res(0, 0, 0);

		Vec3f color = CShaderFlat::shade(ray);

		Vec3f normal = ray.hit->getNormal(ray);									// shading normal
		bool inside = false;
		if (normal.dot(ray.dir) > 0) {
			normal = -normal;													// turn normal to front
			inside = true;
		}

#ifdef DEBUG_MODE
		color = inside ? RGB(1, 0, 0) : RGB(0, 0, 1);
#endif

		// ------ ambient ------
		if (m_ka > 0)
			res += m_ka * m_scene.getAmbientColor().mul(color);

		// ------ diffuse and/or specular ------
		if (m_kd > 0 || m_ke > 0) {
			Ray I(ray.hitPoint());

			for (auto& pLight : m_scene.getLights()) {
				Vec3f L = Vec3f::all(0);
				const size_t nSamples = pLight->getNumSamples();
				for (size_t s = 0; s < nSamples; s++) {
					// get direction to light, and intensity
					I.hit = ray.hit;	// TODO: double check
					auto radiance = pLight->illuminate(I);
					if (radiance && (!pLight->shadow() || !m_scene.if_intersect(I))) {
						// ------ diffuse ------
						if (m_kd > 0) {
							float cosLightNormal = I.dir.dot(normal);
							if (cosLightNormal > 0)
								L += m_kd * cosLightNormal * color.mul(radiance.value());
						}
						// ------ specular ------
						if (m_ks > 0) {
							Vec3f H = normalize(I.dir - ray.dir);
							float cosHalfwayNormal = H.dot(normal);
							if (cosHalfwayNormal > 0)
								L += m_ks * powf(cosHalfwayNormal, m_ke) * radiance.value();
						}
					}
				} // s
				res += (1.0f / nSamples) * L;
			} // pLight
		}
		
		return res;
	}
}
