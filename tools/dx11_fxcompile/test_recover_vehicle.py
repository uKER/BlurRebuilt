"""Small fail-closed and register-mapping checks for retail shader recovery."""
import unittest

from recover_vehicle import recover_shader, sampler_metadata


class RecoveryTests(unittest.TestCase):
    def test_typed_sampler_metadata(self):
        metadata = sampler_metadata("""
            sampler3D DamageVolumeTextureSampler : register(s0) = sampler_state {
                Texture = <g_sVehicleDamageVolumeTexture>;
                MipFilter = LINEAR; MinFilter = LINEAR; MagFilter = LINEAR;
            };
        """)
        assembly = """// DamageVolumeTextureSampler3d s0 1
    vs_3_0
    dcl_position v0
    dcl_volume s0
    dcl_position o0
    texldl r0, v0, s0
    mov o0, r0
"""
        shader, textures = recover_shader(assembly, "DamageVS", metadata)
        self.assertIn("g_sVehicleDamageVolumeTexture.SampleLevel", shader)
        self.assertEqual(textures["s0"][0], "DamageVolumeTextureSampler3d")

    def test_integer_control_points_and_indirect_uniform(self):
        assembly = """// g_vControlPoints c144 18
    vs_3_0
    dcl_texcoord4 v0
    dcl_position o0
    mova a0.xy, v0
    mov o0, c144[a0.x]
"""
        shader, _ = recover_shader(assembly, "ControlVS", {})
        self.assertIn("uint4 v0 : TEXCOORD4", shader)
        self.assertIn("float4 v0 = input.v0", shader)
        self.assertIn("g_vControlPoints[0 + a0.x]", shader)

    def test_masked_write_preserves_full_source_swizzle(self):
        shader, _ = recover_shader("""    vs_3_0
    def c0, 1, 2, 3, 4
    dcl_position v0
    dcl_position o0
    mov r0, v0
    mad r0.xyw, r0.zxyw, c0.x, -r0.xyzz
    mov o0, r0
""", "SwizzleVS", {})
        self.assertIn("r0.xyw = ((r0.zxyw * k0.xxxx + (-r0.xyzz))).xyw", shader)

    def test_unsupported_instruction_fails(self):
        with self.assertRaisesRegex(ValueError, "unsupported instruction"):
            recover_shader("""    vs_3_0
    dcl_position v0
    dcl_position o0
    pow o0, v0, v0
""", "UnsupportedVS", {})

    def test_unmapped_uniform_fails(self):
        with self.assertRaisesRegex(ValueError, "unmapped constant"):
            recover_shader("""    vs_3_0
    dcl_position v0
    dcl_position o0
    mov o0, c42
""", "UnknownVS", {})

    def test_absolute_source_modifier(self):
        shader, _ = recover_shader("""    vs_3_0
    def c0, 1, 2, 3, 4
    dcl_position v0
    dcl_position o0
    mov o0, -c0_abs.x
""", "AbsoluteVS", {})
        self.assertIn("(-abs(k0.xxxx))", shader)


if __name__ == "__main__":
    unittest.main()
