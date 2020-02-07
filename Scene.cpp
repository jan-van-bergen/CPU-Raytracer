#include "Scene.h"

#include "Debug.h"
#include "Spline.h"

#if SCENE == SCENE_DYNAMIC
Scene::Scene() : camera(DEG_TO_RAD(110.0f)), spheres(2), planes(1), sky(DATA_PATH("Sky_Probes/rnl_probe.float")) {
	spheres[0].init(1.0f);
	spheres[1].init(1.0f);
	spheres[0].transform.position = Vector3(-2.0f, 0.0f, 10.0f);
	spheres[1].transform.position = Vector3(+2.0f, 0.0f, 10.0f);
	Material::materials[spheres[0].material_id].diffuse = Vector3(1.0f, 1.0f, 0.0f);
	Material::materials[spheres[1].material_id].diffuse = Vector3(0.0f, 1.0f, 1.0f);
	Material::materials[spheres[0].material_id].reflection = 0.2f;
	Material::materials[spheres[1].material_id].reflection = 0.2f;
	Material::materials[spheres[0].material_id].transmittance = 0.6f;
	Material::materials[spheres[1].material_id].transmittance = 0.6f;
	Material::materials[spheres[0].material_id].index_of_refraction = 1.33f;
	Material::materials[spheres[1].material_id].index_of_refraction = 1.68f;

	planes[0].transform.position.y = -1.0f;
	planes[0].transform.rotation   = Quaternion::axis_angle(Vector3(0.0f, 1.0f, 0.0f), 0.25f * PI);
	Material::materials[planes[0].material_id].texture    = Texture::load(DATA_PATH("Floor.png"));
	Material::materials[planes[0].material_id].reflection = 0.25f;
	
	top_level_bvh.init(6);
	Mesh * diamond   = top_level_bvh.primitives;
	Mesh * monkey    = top_level_bvh.primitives + 1;
	Mesh * icosphere = top_level_bvh.primitives + 2;
	Mesh * rock      = top_level_bvh.primitives + 3;
	Mesh * torus1    = top_level_bvh.primitives + 4;
	Mesh * torus2    = top_level_bvh.primitives + 5;

	diamond->transform.position   = Vector3( 0.0f, 1.0f, 0.0f);
	monkey->transform.position    = Vector3( 4.0f, 2.0f, 0.0f);
	icosphere->transform.position = Vector3( 0.0f, 3.0f, 4.0f);
	rock->transform.position      = Vector3( 6.0f, 4.0f, 4.0f);
	torus1->transform.position    = Vector3( 0.0f, 5.0f, 8.0f);
	torus2->transform.position    = Vector3(-4.0f, 2.0f, 6.0f);

	diamond->init  (DATA_PATH("Diamond.obj"));
	monkey->init   (DATA_PATH("Monkey.obj"));
	icosphere->init(DATA_PATH("icosphere.obj"));
	rock->init     (DATA_PATH("Rock.obj"));
	torus1->init   (DATA_PATH("Torus.obj"));
	torus2->init   (DATA_PATH("Torus.obj"));

	int triangle_count = 0;
	for (int p = 0; p < top_level_bvh.primitive_count; p++) {
		triangle_count += top_level_bvh.primitives[p].bvh->primitive_count;
	}
	printf("Scene contains %i triangles.\n", triangle_count);

	point_lights = new PointLight[point_light_count = 1] {
		PointLight(Vector3(0.0f, 5.0f, 10.0f), Vector3(0.0f, 0.0f, 6.0f))
	};

	spot_lights = new SpotLight[spot_light_count = 1] {
		SpotLight(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 10.0f), Quaternion::axis_angle(Vector3(1.0f, 0.0f, 0.0f), DEG_TO_RAD(70.0f)) * Vector3(0.0f, 0.0f, 1.0f), 70.0f, 80.0f)
	};

	directional_lights = new DirectionalLight[directional_light_count = 1] {
		DirectionalLight(Vector3(0.5f), Vector3::normalize(Vector3(0.0f, -1.0f, 0.0f)))
	};

	camera.position = Vector3(-4.694016f, 6.446100f, -0.572288f);
	camera.rotation = Quaternion(0.268476f, 0.423740f, -0.133092f, 0.854779f);
}
#else

CatmullRomSpline spline_path;

Scene::Scene() : camera(DEG_TO_RAD(110.0f)), spheres(0), planes(0), sky(DATA_PATH("Sky_Probes/rnl_probe.float")) {
	top_level_bvh.init(1);
	top_level_bvh.primitives[0].init(DATA_PATH("sponza/sponza.obj"));

	int triangle_count = 0;
	for (int p = 0; p < top_level_bvh.primitive_count; p++) {
		triangle_count += top_level_bvh.primitives[p].bvh->primitive_count;
	}
	printf("Scene contains %i triangles.\n", triangle_count);

	spline_path.keyframes = new CatmullRomSpline::KeyFrame[spline_path.keyframe_count = 27] {
		{  0.0f, Vector3(   0.000000f,  2.000000f,   0.000000f) },
		{  2.0f, Vector3( -60.612534f, 17.238392f,  15.523449f) },
		{  4.0f, Vector3(-108.123993f, 17.238392f,  -1.894550f) },
		{  6.0f, Vector3(-125.067451f, 17.238392f, -14.969337f) },
		{  8.0f, Vector3(-129.614777f, 17.238392f, -32.944561f) },
		{ 10.0f, Vector3(-115.105927f, 17.238392f, -46.725784f) },
		{ 12.0f, Vector3( -89.085594f, 17.238392f, -52.822845f) },
		{ 14.0f, Vector3( -38.369907f, 17.238392f, -55.176178f) },
		{ 16.0f, Vector3(   2.396957f, 20.345819f, -46.721188f) },
		{ 18.0f, Vector3(  15.381263f, 29.416943f, -37.586372f) },
		{ 20.0f, Vector3(  22.833248f, 31.502077f, -27.499006f) },
		{ 22.0f, Vector3(  26.456038f, 43.059269f, -12.594879f) },
		{ 24.0f, Vector3(  37.350483f, 64.993393f,  17.304836f) },
		{ 26.0f, Vector3(  39.365536f, 65.381531f,  31.403372f) },
		{ 28.0f, Vector3(  39.365536f, 65.381531f,  31.403372f) },
		{ 30.0f, Vector3(  49.182690f, 68.800850f,  37.015442f) },
		{ 32.0f, Vector3(  49.182690f, 68.800850f,  37.015442f) },
		{ 34.0f, Vector3(  85.123070f, 70.012459f,  42.549355f) },
		{ 36.0f, Vector3( 106.102966f, 70.806595f,  27.449497f) },
		{ 38.0f, Vector3( 114.883362f, 72.274231f, -16.361460f) },
		{ 40.0f, Vector3(  93.111549f, 73.381172f, -50.489643f) },
		{ 42.0f, Vector3(  61.455208f, 65.142220f, -27.145912f) },
		{ 44.0f, Vector3(  44.893845f, 88.843460f,  -6.693824f) },
		{ 46.0f, Vector3(  18.025190f, 99.060242f, -13.589282f) },
		{ 48.0f, Vector3(  17.637579f, 99.073540f, -13.557362f) },
		{ 50.0f, Vector3(   8.432379f, 83.863182f, -11.071933f) },
		{ 52.0f, Vector3(  12.578653f, 37.367577f,   1.379128f) }
	};
	
	point_light_count = 0;
	spot_light_count  = 0;

	directional_lights = new DirectionalLight[directional_light_count = 1] {
		DirectionalLight(Vector3(0.9f), Vector3::normalize(Vector3(0.0f, -1.0f, 0.0f)))
	};
	
	camera.position = Vector3(0.0f, 10.0f, 0.0f);
	camera.rotation = Quaternion::axis_angle(Vector3(0.0f, 1.0f, 0.0f), DEG_TO_RAD(-90.0f));
}
#endif

Scene::~Scene() {
	delete [] point_lights;
	delete [] spot_lights;
	delete [] directional_lights;
}

void Scene::update(float delta) {
#if SCENE == SCENE_DYNAMIC
	top_level_bvh.primitives[0].transform.rotation = Quaternion::axis_angle(Vector3(0.0f, 1.0f, 0.0f), delta) * top_level_bvh.primitives[0].transform.rotation;

	static float time = 0.0f;
	time += delta;

	top_level_bvh.primitives[1].transform.position.y = 1.0f + 2.0f * sinf(time);

	top_level_bvh.primitives[2].transform.position.x -= delta * 0.5f;

	top_level_bvh.primitives[3].transform.position = Vector3(6.0f, 4.0f + 2.0f * sinf(time*0.5f), 4.0f + 2.0f *cosf(time*0.5f));
	top_level_bvh.primitives[3].transform.rotation = Quaternion::axis_angle(Vector3(0.0f, 1.0f, 0.0f), delta * 0.5f) * top_level_bvh.primitives[3].transform.rotation;

	top_level_bvh.primitives[4].transform.rotation = Quaternion::axis_angle(Vector3(1.0f, 0.0f, 0.0f), delta) * top_level_bvh.primitives[4].transform.rotation;

	top_level_bvh.primitives[5].transform.rotation = Quaternion::nlerp(Quaternion(), Quaternion::axis_angle(Vector3(1.0f, 0.0f, 0.0f), DEG_TO_RAD(-90.0f)), 0.5f + 0.5f*sinf(time));
#else
	//Vector3 prev_camera_position = camera.position;
	//camera.position = spline_path.get_point(delta);

	//Vector3 forward = camera.position - prev_camera_position;
	//Vector3 up      = Vector3(0.0f, 1.0f, 0.0f);
	//camera.rotation = Quaternion::look_rotation(forward, up);
#endif
	
	camera.update(delta, SDL_GetKeyboardState(0));

	spheres.update();
	planes.update();
	top_level_bvh.update();
	top_level_bvh.build_bvh();
}

void Scene::trace_primitives(const Ray & ray, RayHit & ray_hit) const {
	spheres.trace(ray, ray_hit);
	planes.trace(ray, ray_hit);
	top_level_bvh.trace(ray, ray_hit);
}

SIMD_float Scene::intersect_primitives(const Ray & ray, SIMD_float max_distance) const {
	SIMD_float result(0.0f);
	
	result = spheres.intersect(ray, max_distance);
	if (SIMD_float::all_true(result)) return result;

	result = result | planes.intersect(ray, max_distance);
	if (SIMD_float::all_true(result)) return result;

	result = result | top_level_bvh.intersect(ray, max_distance);
	return result;
}
