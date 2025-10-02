# RockPi-S Pinout (Hardware v13)

Este documento muestra el pinout de la RockPi-S (hardware v13).

📖 Fuente oficial: [Radxa Wiki](https://wiki.radxa.com/RockpiS/hardware/gpio)

---

<div class="panel-body">
<h4><span class="mw-headline" id="26-pin_Header_1_4">26-pin Header 1</span></h4>
<table class="mw-collapsible wikitable">
<tbody><tr>
<th> GPIO number </th>
<th> Func4 </th>
<th> Func3 </th>
<th> Func2 </th>
<th>  Func1 </th>
<th>  Pin# </th>
<th> </th>
<th> Pin# </th>
<th> Func1 </th>
<th> Func2 </th>
<th> Func3 </th>
<th> Func4 </th>
<th> GPIO number </th>
</tr>
<tr>
<td> </td><td> </td><td> </td><td> </td><td> +3.3V </td>
<td style="background:yellow"> 1 </td>
<td> </td>
<td style="background:red"> 2 </td>
<td> +5.0V </td>
<td> </td><td> </td><td> </td><td> </td>
</tr>
<tr>
<td> 11 </td><td> </td><td> </td><td> I2C1_SDA </td>
<td> GPIO0_B3 </td>
<td style="background:green; color:white"> 3 </td>
<td> </td>
<td style="background:red"> 4 </td>
<td> +5.0V </td>
<td> </td><td> </td><td> </td><td> </td>
</tr>
<tr>
<td> 12 </td><td> </td><td> </td><td> I2C1_SCL </td>
<td> GPIO0_B4 </td>
<td style="background:green; color:white"> 5 </td>
<td> </td>
<td style="background:black; color:white"> 6 </td>
<td> GND </td>
<td> </td><td> </td><td> </td><td> </td>
</tr>
<tr>
<td> 68 </td><td> </td><td> PDM_CLK_M_M2 </td><td> I2S0_8CH_MCLK </td>
<td> GPIO2_A4 </td>
<td style="background:green; color:white"> 7 </td>
<td> </td>
<td style="background:green; color:white"> 8 </td>
<td> GPIO2_A1 </td>
<td style="background:orange">UART0_TX </td>
<td> SPI0_MOSI  </td>
<td> </td><td> 65 </td>
</tr>
<tr>
<td> </td><td> </td><td> </td><td> </td><td> GND </td>
<td style="background:black; color:white"> 9 </td>
<td> </td>
<td style="background:green; color:white">10 </td>
<td> GPIO2_A0 </td>
<td style="background:orange">UART0_RX </td>
<td> SPI0_MISO  </td>
<td> </td><td> 64 </td>
</tr>
<tr>
<td> 15 </td><td> </td><td> I2C3_SDA_M0 </td><td> PWM2 </td>
<td> GPIO0_B7 </td>
<td style="background:green; color:white">11 </td>
<td> </td>
<td style="background:green; color:white">12 </td>
<td> GPIO2_A5 </td>
<td> I2S0_8CH_SCLK_TX </td>
<td> </td><td> </td><td> 69 </td>
</tr>
<tr>
<td> 16 </td><td> </td><td> I2C3_SCL_M0 </td><td> PWM3 </td>
<td> GPIO0_C0 </td>
<td style="background:green; color:white">13 </td>
<td> </td>
<td style="background:black; color:white">14 </td>
<td> GND </td>
<td> </td><td> </td><td> </td><td> </td>
</tr>
<tr>
<td> 17 </td><td> </td><td> </td><td> SPDIF_TX </td>
<td> GPIO0_C1 </td>
<td style="background:green; color:white">15 </td>
<td> </td>
<td style="background:green; color:white">16 </td>
<td> GPIO2_B2 </td>
<td> I2S0_8CH_SDO1 </td>
<td> </td><td> </td><td> 74 </td>
</tr>
<tr>
<td> </td><td> </td><td> </td><td> </td><td> +3.3V </td>
<td style="background:yellow">17 </td>
<td> </td>
<td style="background:green; color:white">18 </td>
<td> GPIO2_B1 </td>
<td> I2S0_8CH_SDO0 </td>
<td> </td><td> </td><td> 73 </td>
</tr>
<tr>
<td> 55 </td><td> SPI2_MOSI </td><td> UART2_TX_M0 </td><td> UART1_RTSN </td>
<td> GPIO1_C7 </td>
<td style="background:green; color:white">19 </td>
<td> </td>
<td style="background:black; color:white">20 </td>
<td> GND </td>
<td> </td><td> </td><td> </td><td> </td>
</tr>
<tr>
<td> 54 </td><td> SPI2_MISO </td><td> UART2_RX_M0 </td><td> UART1_CTSN </td>
<td> GPIO1_C6 </td>
<td style="background:green; color:white">21 </td>
<td> </td>
<td style="background:green; color:white">22 </td>
<td> GPIO2_A7 </td>
<td> I2S0_8CH_LRCK_TX </td>
<td> </td><td> </td><td> 71 </td>
</tr>
<tr>
<td> 56 </td><td> SPI2_CLK </td><td> I2C0_SDA </td><td> UART1_RX </td>
<td> GPIO1_D0 </td>
<td style="background:green; color:white">23 </td>
<td> </td>
<td style="background:green; color:white">24 </td>
<td> GPIO1_D1 </td>
<td> UART1_TX </td>
<td> I2C0_SCL </td>
<td> SPI2_CSN0 </td>
<td> 57 </td>
</tr>
<tr>
<td> </td><td> </td><td> </td><td> </td><td> GND </td>
<td style="background:black; color:white">25 </td>
<td> </td>
<td style="background:green; color:white">26 </td>
<td> ADC_IN0 </td>
<td> </td><td> </td><td> </td><td> </td>
</tr>
</tbody></table>

<h4><span class="mw-headline" id="26-pin_Header_2_4">26-pin Header 2</span></h4>
<table class="mw-collapsible wikitable">
<tbody>
<tr>
<th> GPIO number </th>
<th> Func4 </th>
<th> Func3 </th>
<th> Func2 </th>
<th> Func1 </th>
<th> Pin# </th>
<th> </th>
<th> Pin# </th>
<th> Func1 </th>
<th> Func2 </th>
<th> Func3 </th>
<th> Func4 </th>
<th> GPIO number </th>
</tr>
<tr>
<td> </td><td> </td><td> </td><td> </td><td> GND </td>
<td style="background:black; color:white">27 </td>
<td> </td>
<td style="background:black; color:white">28 </td>
<td> GPIO2_B5 </td>
<td> I2S0_8CH_SDI0 </td>
<td> PDM_SDI0_M2 </td>
<td> </td><td> 77 </td>
</tr>
<tr>
<td> </td><td> </td><td> </td><td> </td><td> ADC_KEY_IN1 </td>
<td style="background:black; color:white">29 </td>
<td> </td>
<td style="background:black; color:white">30 </td>
<td> GPIO2_B6 </td>
<td> I2S0_8CH_SDI1 </td>
<td> PDM_SDI1_M2 </td>
<td> </td><td> 78 </td>
</tr>
<tr>
<td> </td><td> </td><td> </td><td> </td><td> MICBIAS2 </td>
<td style="background:black; color:white">31 </td>
<td> </td>
<td style="background:black; color:white">32 </td>
<td> GPIO2_B7 </td>
<td> I2S0_8CH_SDI2 </td>
<td> PDM_SDI2_M2 </td>
<td> </td><td> 79 </td>
</tr>
<tr>
<td> </td><td> </td><td> </td><td> </td><td> MICBIAS1 </td>
<td style="background:black; color:white">33 </td>
<td> </td>
<td style="background:black; color:white">34 </td>
<td> GPIO2_C0 </td>
<td> I2S0_8CH_SDI3 </td>
<td> PDM_SDI3_M2 </td>
<td> </td><td> 80 </td>
</tr>
<tr>
<td> </td><td> </td><td> </td><td> </td><td> MICN8 </td>
<td style="background:black; color:white">35 </td>
<td> </td>
<td style="background:black; color:white">36 </td>
<td> MCIP8 </td>
<td> </td><td> </td><td> </td><td> </td>
</tr>
<tr>
<td> </td><td> </td><td> </td><td> </td><td> MICN7 </td>
<td style="background:black; color:white">37 </td>
<td> </td>
<td style="background:black; color:white">38 </td>
<td> MCIP7 </td>
<td> </td><td> </td><td> </td><td> </td>
</tr>
<tr>
<td> 109 </td><td> UART3_TX </td><td> I2C3_SCL_M1 </td><td> SPI1_CSN0 </td>
<td> GPIO3_B5 </td>
<td style="background:black; color:green">39 </td>
<td> </td>
<td style="background:black; color:green">40 </td>
<td> GPIO3_B4 </td>
<td> SPI1_MOSI </td>
<td> I2C3_SDA_M1 </td>
<td> UART3_RX </td><td> 108 </td>
</tr>
<tr>
<td> 107 </td><td> </td><td> </td><td> SPI1_CLK </td>
<td> GPIO3_B3 </td>
<td style="background:black; color:green">41 </td>
<td> </td>
<td style="background:black; color:green">42 </td>
<td> GPIO3_B2 </td>
<td> SPI1_MISO </td>
<td> </td><td> </td><td> 106 </td>
</tr>
<tr>
<td> 76 </td><td> </td><td> </td><td> I2S0_8CH_SDO3 </td>
<td> GPIO2_B4 </td>
<td style="background:black; color:white">43 </td>
<td> </td>
<td style="background:black; color:white">44 </td>
<td> GPIO2_B3 </td>
<td> I2S0_8CH_SDO2 </td>
<td> </td><td> </td><td> 75 </td>
</tr>
<tr>
<td> 72 </td><td> </td><td> </td><td> I2S0_8CH_LRCK_RX </td>
<td> GPIO2_B0 </td>
<td style="background:black; color:white">45 </td>
<td> </td>
<td style="background:black; color:white">46 </td>
<td> GPIO2_A6 </td>
<td> I2S0_8CH_SCLK_RX </td>
<td> PDM_CLK_S_M2 </td>
<td> </td><td> 70 </td>
</tr>
<tr>
<td> </td><td> </td><td> </td><td> </td><td> MICN2 </td>
<td style="background:black; color:white">47 </td>
<td> </td>
<td style="background:black; color:white">48 </td>
<td> MCIP2 </td>
<td> </td><td> </td><td> </td><td> </td>
</tr>
<tr>
<td> </td><td> </td><td> </td><td> </td><td> MICN1 </td>
<td style="background:black; color:white">49 </td>
<td> </td>
<td style="background:black; color:white">50 </td>
<td> MCIP1 </td>
<td> </td><td> </td><td> </td><td> </td>
</tr>
<tr>
<td> </td><td> </td><td> </td><td> </td><td> LINEOUT_R </td>
<td style="background:black; color:white">51 </td>
<td> </td>
<td style="background:black; color:white">52 </td>
<td> LINEOUT_L </td>
<td> </td><td> </td><td> </td><td> </td>
</tr>
</tbody></table>
</div>

# Extra Details (Hardware v13)

<!-- ===== EXTRA DETAILS ===== -->

<h4><span class="mw-headline" id="More_details_about_V13_Headers">More details about V13 Headers</span></h4>
<ul>
<li> The difference between V12 and V13 is determined by PIN#13/14/15/16 on 26-pin Header 2.
</li>
<li> Pins marked with color orange are designed for default debug console.
</li>
<li> PWM: x2, PWM2/PWM3
</li>
<li> SPI: x2, SPI1, SPI2
</li>
<li> I2C: x3, I2C0/I2C1/I2C3
</li>
<li> UART: x4, UART0/UART1/UART2/UART3
</li>
<li> ADC: x1, ADC_IN0. The max input voltage is 1.8V.
</li>
<li> When I2C3 is used, you should only select one of the two groups, PIN#11/13 on 26-pin Header 1 and PIN#13/14 on 26-pin Header 2.
</li>
</ul>

<h2><span class="mw-headline" id="IO_Voltage">IO Voltage</span></h2>
<p>RK3308 have two IO voltages, <b>1.8V/3.3V</b>. For ROCK Pi S, the voltage level of GPIOs showed in the tables above are <b>3.3V</b> and tolerance of those  are <b>3.63V</b>. For hardware V11 and V12, an ADC input is included (ADC_IN0). This ADC has an input voltage range of <b>0-1.8V</b>.
</p>

<h2><span class="mw-headline" id="GPIO_number">GPIO number</span></h2>
<p>Rockchip RK3308 GPIO has 5 banks, GPIO0 to GPIO4, each bank has 32pins, naming as below:
</p>
<pre>   GPIO0_A0 ~ A7 
   GPIO0_B0 ~ B7
   GPIO0_C0 ~ C7
   GPIO0_D0 ~ D7

GPIO1_A0 ~ A7
....
GPIO1_D0 ~ D7
</pre>
<p>For Rockchip 4.4 kernel, the GPIO number can be calculated as below, take GPIO4_D3(PIN8 on 26PIN GPIO) as an example:
</p>
<pre>   GPIO4_D3 = 32*4 + 8*3 + 3 = 155
</pre>
<p>To set GPIO4_D3 output
</p>
<pre>   cd /sys/class/gpio
   echo 155 > export
   cd gpio155
   echo out > direction
   echo 1 > value     # output high
   echo 0 > value     # output low
</pre>

<h2><span class="mw-headline" id="Power_supply">Power supply</span></h2>
<p>1.The pins, <code>PIN#2 and PIN#4</code>, can supply power of <code>5V</code>. The maximum current that can output depends on the power adapter.
</p><p>2.The pins, <code>PIN#1 and PIN#17</code>, can supply power of <code>3.3V</code>. The maximum current that can output is <code>200mA</code>.
</p>