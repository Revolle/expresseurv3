
-- une transaction :
--     sélection au hasard d'un compte solbale A ( > 0 )
--     recherche d'un voisin B avec un compte solvable
--     A et B misent chacun un pourcentage du plus faible des deux comptes ( <enjeu> % )
--     A ou B remporte les deux mises , au hasard
--     une taxe sur le gain est redistibuée à tous ( <taxegain> %  des deux mises gagn&es)
--     une taxe sur le compte est redistribuée à tous ( <taxepatrimoine> % du capital au-dessus de la valeur initiale <v0> )
local nbtransaction = 10000 -- nombre de transactions à simuler
local largeur = 10 -- longeur du carré de joueurs
local v0 = 100.0 -- capital initial distribué à chaque joueur
local enjeu = 50.0 -- pourcentage misé, sur le plus pauvre des deux joueurs en transaction
local taxegain = 0.0 -- pourcentage taxe sur le gain, redistribué après transaction
local taxepatrimoine = 1.0 -- pourcentage taxe sur le capital au-dessus de <v0>, redistribué après transaction
local decilemax = 2 * v0 -- deciles sur <decilemax> fois <v0>
local credit = {} -- tableau des credits ar joueur
local debug = false

function initV()
  for i = 1, largeur  do
      credit[i] = {}
      for j = 1, largeur do
        credit[i][j] = v0
      end
  end
end
function displayV()
  local s 
  local total = 0.0 
  local max = 0.0 
  local decile = {}
  for j = 1, largeur  do
    s = ""
    for i = 1, largeur do
      s = s .. string.format("%8d",math.floor(credit[i][j]))
      total = total + credit[i][j]
      if credit[i][j] > max then
        max = credit[i][j]
      end
    end
    print(s)
  end
  print("total",total)
  print("max",max)
  for d = 1,10 do
    decile[d] = 0 
  end
  max = max + 1
  for i = 1 , largeur do
    for j = 1 , largeur do
      d = math.floor(10 * credit[i][j] / decilemax) + 1
      if d > 10 then
        d = 10 
      end
      if d < 1 then
        d = 1 
      end
      decile[d] = decile[d] + 1
    end
  end
  for d = 1,10 do
    lg = math.floor(40 * decile[d] / (largeur * largeur ))
    per = math.floor( 100 * decile[d] /( largeur * largeur))
    ver = math.floor( (d - 1)* decilemax / 10 )
    ser = string.format("%5d %5d",ver,per)
    if lg > 80 then
      print(ser , string.rep("!",100) )
    else
      print(ser , string.rep("*",lg + 1) )
    end
  end
end
function transaction()
  
  -- selection joueurs
  local pi1, pi2, pj1, pj2
  pi1 = math.random (1,largeur)
  pj1 = math.random (1,largeur)
  if credit[pi1][pj1] < 1.0 then
    return
  end
  if math.random (1,2) == 1 then
    -- voisin horizontal
    pj2 = pj1
    if math.random (1,2) == 1 then
      -- voisin droite
      --print("droite")
      pi2 = pi1
      while true do
        pi2 = pi2 + 1
        if pi2 > largeur then
          pi2 = 1
        end
        if credit[pi2][pj2] > 1.0 then
          break 
        end
        if pi2 == pi1 then
          return
        end
      end
    else
      -- voisin gauche
      --print("gauche")
      pi2 = pi1
      while true do
        pi2 = pi2 - 1
        if pi2 < 1 then
          pi2 = largeur
        end
        if credit[pi2][pj2] > 1.0 then
          break 
        end
        if pi2 == pi1 then
          return
        end
      end
   end
  else
    -- voisin vertical
    pi2 = pi1
    if math.random (1,2) == 1 then
      -- voisin bas
      --print("bas")
      pj2 = pj1
      while true do
        pj2 = pj2 + 1
        if pj2 > largeur then
          pj2 = 1
        end
        if credit[pi2][pj2] > 1.0 then
          break 
        end
        if pj2 == pj1 then
          return
        end
      end
    else
      -- voisin haut
      --print("haut")
      pj2 = pj1
      while true do
        pj2 = pj2 - 1
        if pj2 < 1 then
          pj2 = largeur
        end
        if credit[pi2][pj2] > 1.0 then
          break 
        end
        if pj2 == pj1 then
          return
        end
      end
    end
  end

  
  -- jeu entre les deux joueurs
  mise = math.min(credit[pi1][pj1] , credit[pi2][pj2] ) * enjeu / 100
  gagnant = 2 * mise 
  perdant = 2 * mise - gagnant 
  taxeplusvalue = (gagnant - mise) * taxegain / 100
  redistribution = taxeplusvalue / ( largeur * largeur )
  if debug then
    print("p1",pi1,pj1)
    print("p2",pi2,pj2)    
    print("mise",mise)
    print("gagnant",gagnant)
    print("perdant",perdant)
    print("taxeplusvalue", taxeplusvalue)
    print("redistribution",redistribution)
  end
  if math.random (1,2) == 1 then
    credit[pi1][pj1] = credit[pi1][pj1] - mise + gagnant - taxeplusvalue
    credit[pi2][pj2] = credit[pi2][pj2] - mise + perdant
  else
    credit[pi1][pj1] = credit[pi1][pj1] - mise + perdant
    credit[pi2][pj2] = credit[pi2][pj2] - mise + gagnant - taxeplusvalue
  end
  
  -- redistribution taxe sur le gain pour tout le monde
  impot = 0.0
  for i = 1, largeur  do
      for j = 1, largeur do
        credit[i][j] = credit[i][j] + redistribution
        if credit[i][j] > v0 then
          impot = impot + ( credit[i][j] - v0 ) * taxepatrimoine / 100
          credit[i][j] = credit[i][j] - ( credit[i][j] - v0 ) * taxepatrimoine / 100
        end
      end
  end
  -- redistribution impot
  redistribution = impot / ( largeur * largeur )
  if debug then
    print("impot", impot)
    print("redistribution",redistribution)
  end
  for i = 1, largeur  do
      for j = 1, largeur do
        credit[i][j] = credit[i][j] + redistribution
      end
  end

end

math.randomseed(os.time())
initV()
displayV()
for n = 1 , nbtransaction do
  transaction()
  if debug then
    displayV()
  end
end
displayV()