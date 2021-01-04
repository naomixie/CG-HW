#version 330 core
out vec4 FragColor;

//如果我们打算从一个着色器向另一个着色器发送数据，我们必须在发送方着色器中声明一个输出，
//在接收方着色器中声明一个类似的输入。当类型和名字都一样的时候，
//OpenGL就会把两个变量链接到一起，它们之间就能发送数据了（这是在链接程序对象时完成的）。
in vec4 vPos;       //vPOS 为 vs.shader的out变量
in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 objectColor;
uniform vec3 lightColor;


uniform sampler2D texture_diffuse1;

// 片段着色器只需要一个输出变量，这个变量是一个4分量向量，它表示的是最终的输出颜色，
// 我们应该自己将其计算出来。我们可以用out关键字声明输出变量，这里我们命名为FragColor。
void main()
{    
    // // FragColor = vPos;
    // // FragColor = vec4(0.5, 0.0, 0.0, 1.0);
    // // glm::vec4 lightColor(1.0,1.0,1.0,1.0);
    // // glm::vec4 toyColor(1.0,0.5,0.31,1.0);
    // // FragColor = vPos*vec4(0.5, 0.0, 0.0, 1.0);
    // // FragColor = vec4(1.0,1.0,1.0,1.0) * vec4(1.0,0.5,0.31,1.0) ;
    // // FragColor = lightColor * toyColor;
    // // FragColor = vec4(lightColor * objectColor,1.0);
    
    // // Calculating Ambient Lighting
    // float ambientStrength = 0.1;
    // vec3 ambient = ambientStrength * lightColor;

    // // vec3 result = ambient * objectColor;
    // // FragColor = vec4(result, 1.0);

    // // Calculating Diffuse Lighting
    // // The first thing we need to calculate is the direction vector between the light source and the fragment's position.
    // // We can easily calculate this difference by subtracting both vectors from each other.
    // vec3 norm = normalize(Normal);
    // vec3 lightDir = normalize(lightPos - FragPos);

    // // Next we need to calculate the diffuse impact of the light on the current fragment 
    // // by taking the dot product between the norm and lightDir vectors.

    // //If the angle between both vectors is greater than 90 degrees 
    // // then the result of the dot product will actually become negative and we end up with a negative diffuse component. 
    // // For that reason we use the max function that returns the highest of both its parameters to make sure the diffuse component (and thus the colors) never become negative. 
    // float diff = max(dot(norm, lightDir), 0.0);     
    // vec3 diffuse = diff * lightColor;

    // // Now that we have both an ambient and a diffuse component we add both colors to each other 
    // // and then multiply the result with the color of the object to get the resulting fragment's output color:

    // // vec3 result = (ambient + diffuse) * objectColor;
    // // FragColor = vec4(result, 1.0);

    // // Specular Lighting
    // // Similar to diffuse lighting, specular lighting is based on the light's direction vector and the object's normal vectors, 
    // // but this time it is also based on the view direction e.g. from what direction the player is looking at the fragment.

    // // We calculate a reflection vector by reflecting the light direction around the normal vector. 
    // // Then we calculate the angular distance between this reflection vector and the view direction. 

    // float specularStrength = 0.5;
    // vec3 viewDir = normalize(viewPos - FragPos);
    // // Note that we negate the lightDir vector. The reflect function expects the first vector to point 
    // // from the light source towards the fragment's position, but the lightDir vector is currently pointing the other way around:
    // // from the fragment towards the light source (this depends on the order of subtraction earlier on when we calculated the lightDir vector). 
    // // To make sure we get the correct reflect vector we reverse its direction by negating the lightDir vector first.
    // // The second argument expects a normal vector so we supply the normalized norm vector.
    // vec3 reflectDir = reflect(-lightDir, norm);  
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // vec3 specular = specularStrength * spec * lightColor;  

    // vec3 result = (ambient + diffuse + specular) * objectColor;
    // FragColor = vec4(result, 1.0);
    // FragColor = vec4(lightColor*objectColor,1.0);
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  
        
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}