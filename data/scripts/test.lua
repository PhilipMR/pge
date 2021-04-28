function subroutine()
	print("Hello, World!")
end

function onUpdate(dt)
	if testIsQButtonDown() then
		subroutine()
	end
end





