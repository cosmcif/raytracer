# Raytracer 
USI Computer Graphics rendering competition 2023 (we won :p). All results [here](https://www.pdf.inf.usi.ch/rendering_competition/2023/).

Run the code with
`g++ main.cpp -Ofast; ./a.out`
## Authors
- Sofia d'Atri
- Nicolò Tafta

<details open>
 <summary><h1>Final scene<h1></summary>
<img src="https://github.com/cosmcif/raytracer/assets/75504103/d77fe0a4-1197-4919-a06a-1219e2d8bb99">
</details>

<details>
 <summary><h1>Material sample scene<h1></summary>
<img src="https://github.com/cosmcif/raytracer/assets/75504103/4b64640c-702c-42d0-a257-ac6accd2c3e5">

</details>

# Features implemented
Features are highlighted in the files and can be found by searching `FEAT` (more specific feature tags at end of file)
- [x] **Stochastic Raytracer - Antialiasing**
- [x] **Perlin Noise - Textures**
- [x] **Perlin Noise - Normal maps**
- [x] **Advanced Reflectance Model**
- [x] **Scene**

<details>
<summary><h1>Antialiasing<h1></summary>
<img src="https://github.com/cosmcif/raytracer/assets/75504103/81265689-4312-4c15-8802-5d6f16b9c6b3">
</details>
<details>
<summary><h1>Perlin noise - Textures<h1></summary>
<img src="https://github.com/cosmcif/raytracer/assets/75504103/ade387d1-726b-4822-a121-2872b1f56d56">
</details>
<details>
<summary><h1>Perlin noise - Normal maps<h1></summary>
<img src="https://github.com/cosmcif/raytracer/assets/75504103/7c0042fe-fb99-497f-a3e3-70e59a91680a">
<img src="https://github.com/cosmcif/raytracer/assets/75504103/accaf57d-66b0-4c3e-b95e-c6ce377aea81">
<img src="https://github.com/cosmcif/raytracer/assets/75504103/177f1f15-2fbf-43fb-b348-0c6d5f242916">
</details>
<details>
<summary><h1>Specular highlights<h1></summary>
<img src="https://github.com/cosmcif/raytracer/assets/75504103/7fc595c1-2558-4947-a045-1206eea228e0">
</details>
      
# How to locate features in the code
Features can be found by looking for these comments (or just `FEAT`)
- FEAT: BOUNDING VOLUME HIERARCHY (BVH)
- FEAT: IMAGE TEXTURES
- FEAT: MESH LOADER
- FEAT: NORMAL MAPS
- FEAT: PERLIN GENERATED NORMAL MAPS
- FEAT: PERLIN GENERATED TEXTURES
- FEAT: SPECULAR HIGHLIGHTS
- FEAT: SUPER SAMPLING ANTI ALIASING (SSAA)
